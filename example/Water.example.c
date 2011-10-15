#include "Common.h"
#include "Remote.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"
#include "Pvr.h"
#include "red_tile_texture.h"

#include "../lib/crender/Mem.h"
#include "../lib/crender/Texture.h"

AppContext* app = nullptr;
//RemoteConfig* config = nullptr;

Mesh* floorMesh = nullptr;
Mesh* waterMesh = nullptr;
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* waterMtl = nullptr;
Material* bgMtl = nullptr;
CrTexture* texture = nullptr;
CrTexture* refractTex = nullptr;
CrTexture* rttDepth = nullptr;

typedef struct Settings
{
	float shadowSlopScale;
} Settings;

Settings settings = {4};

typedef struct Input
{
	CrBool isDown;
	int x;
	int y;
	
} Input;

Input input = {0};

typedef enum WaterBuffer
{
	WaterBuffer_Position0,
	WaterBuffer_Position1,
	WaterBuffer_Count,
} WaterBuffer;

typedef enum WaterMaterial
{
	WaterMaterial_Init,
	WaterMaterial_Step,
	WaterMaterial_AddDrop,
	WaterMaterial_Count,

} WaterMaterial;

typedef struct Water
{
	size_t size;
	size_t curr;
	CrBool inited;
	Mesh* screenQuad;
	CrTexture* buffers[WaterBuffer_Count];
	Material* materials[WaterMaterial_Count];
	CrSampler psampler;
	
} Water;

Water* water;
const CrVec3 waterN = {0.0f, 1.0f, 0.0f};
const CrVec3 waterP = {0.0f, 0.0f, 0.0f};
const float waterSize = 3.0f;

Water* waterNew(size_t size)
{
	size_t i;
	Water* self = crMem()->alloc(sizeof(Water), "water");
	memset(self, 0, sizeof(Water));

	crDbgStr("create water position buffers\n");

	self->size = size;
	for(i=0; i<2; ++i) {
		size_t id = WaterBuffer_Position0 + i;
		self->buffers[id] = crTextureAlloc();
		//crTextureInitRtt(self->buffers[id], size, size, 0, 1, CrGpuFormat_FloatR16G16B16A16);
		crTextureInitRtt(self->buffers[id], size, size, 0, 1, CrGpuFormat_UnormR8G8B8A8);
	}

	crDbgStr("load water materials quad\n");

	appLoadMaterialBegin(app, nullptr);

	self->materials[WaterMaterial_Init] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.Init.Fragment",
		nullptr, nullptr, nullptr);

	self->materials[WaterMaterial_Step] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.Step.Fragment",
		nullptr, nullptr, nullptr);

	self->materials[WaterMaterial_AddDrop] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.AddDrop.Fragment",
		nullptr, nullptr, nullptr);

	self->psampler.filter = CrSamplerFilter_MagMin_Nearest_Mip_None;
	self->psampler.addressU = CrSamplerAddress_Clamp;
	self->psampler.addressV = CrSamplerAddress_Clamp;
	self->psampler.addressW = CrSamplerAddress_Clamp;

	appLoadMaterialEnd(app);

	crDbgStr("create water screen quad\n");

	self->screenQuad = meshAlloc();
	meshInitWithScreenQuad(self->screenQuad);

	crDbgStr("water new complete\n");

	self->inited = CrFalse;

	return self;
}

void waterFree(Water* self)
{
	size_t i;
	for(i=0; i<WaterBuffer_Count; ++i) {
		crTextureFree(self->buffers[i]);
	}
	for(i=0; i<WaterMaterial_Count; ++i) {
		materialFree(self->materials[i]);
	}
	meshFree(self->screenQuad);
	crMem()->free(self, "water");
}

void waterPreProcess(Water* self, CrTexture* target)
{
	CrTexture* bufs[] = {target, nullptr};

	crContextPreRTT(crContext(), bufs, nullptr);
	crContextSetViewport(crContext(), 0, 0, (float)target->width, (float)target->height, -1, 1);

	crContext()->gpuState.depthTest = CrFalse;
	crContext()->gpuState.depthWrite = CrFalse;
	crContextApplyGpuState(crContext());
}

void waterPostProcess(Water* self)
{
	crContextPostRTT(crContext());
	crContextSetViewport(crContext(), 0, 0, (float)crContext()->xres, (float)crContext()->yres, -1, 1);

	crContext()->gpuState.depthTest = CrTrue;
	crContext()->gpuState.depthWrite = CrTrue;
}

size_t waterCurrBuffer(Water* self)
{
	return self->curr;
}

size_t waterLastBuffer(Water* self)
{
	return (self->curr == 0) ? 1 : 0;
}

void waterSwapBuffers(Water* self)
{
	self->curr = waterLastBuffer(self);
}

void waterInit(Water* self)
{
	size_t i=0;

	if(nullptr ==self->screenQuad)
		return;

	for(i=0; i<2; ++i) {
		CrGpuProgram* prog = self->materials[WaterMaterial_Init]->program;

		waterPreProcess(self, self->buffers[WaterBuffer_Position0+i]);

		crGpuProgramPreRender(prog);
		meshPreRender(self->screenQuad, prog);
		meshRenderTriangles(self->screenQuad);
	}

	waterPostProcess(self);

	self->curr = 0;
	self->inited = CrTrue;
}

void waterStep(Water* self)
{
	size_t curr = WaterBuffer_Position0 + waterCurrBuffer(self);
	size_t last = WaterBuffer_Position0 + waterLastBuffer(self);

	CrGpuProgram* prog = self->materials[WaterMaterial_Step]->program;

	if(nullptr ==self->screenQuad)
		return;

	waterPreProcess(self, self->buffers[curr]);

	crGpuProgramPreRender(prog);
	{ float val[] = {1.0f / self->size, 1.0f / self->size};
	crGpuProgramUniform2fv(prog, CrHash("u_delta"), 1, val); }
	
	crGpuProgramUniformTexture(prog, CrHash("u_buffer"), self->buffers[last], &self->psampler);

	meshPreRender(self->screenQuad, prog);
	meshRenderTriangles(self->screenQuad);

	waterPostProcess(self);

	waterSwapBuffers(self);
}

void waterAddDrop(Water* self, float x, float y, float r, float s)
{
	size_t curr = WaterBuffer_Position0 + waterCurrBuffer(self);
	size_t last = WaterBuffer_Position0 + waterLastBuffer(self);

	CrGpuProgram* prog = self->materials[WaterMaterial_AddDrop]->program;

	if(nullptr ==self->screenQuad)
		return;

	waterPreProcess(self, self->buffers[curr]);

	crGpuProgramPreRender(prog);
	{ float val[] = {x, y};
	crGpuProgramUniform2fv(prog, CrHash("u_center"), 1, val); }

	crGpuProgramUniform1fv(prog, CrHash("u_radius"), 1, &r);

	crGpuProgramUniform1fv(prog, CrHash("u_strength"), 1, &s);

	crGpuProgramUniformTexture(prog, CrHash("u_buffer"), self->buffers[last], &self->psampler);

	meshPreRender(self->screenQuad, prog);
	meshRenderTriangles(self->screenQuad);

	waterPostProcess(self);

	waterSwapBuffers(self);
}

void drawBackground()
{
	static const CrVec4 c[] = {
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.145f, 0.31f, 0.405f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
	};
	CrGpuState* gpuState = &crContext()->gpuState;

	gpuState->depthTest = CrFalse;
	gpuState->cull = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(bgMtl->program);
	crGpuProgramUniform4fv(bgMtl->program, CrHash("u_colors"), 4, (const float*)c);

	meshPreRender(bgMesh, bgMtl->program);
	meshRenderTriangles(bgMesh);
}

void drawScene(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx, CrVec3 camPos)
{
	CrGpuProgram* prog = sceneMtl->program;
	CrGpuState* gpuState = &crContext()->gpuState;

	gpuState->cull = CrFalse;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(prog);
	{	
		CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Wrap, CrSamplerAddress_Wrap};
		crGpuProgramUniformTexture(prog, CrHash("u_tex"), texture, &sampler);
	}

	crGpuProgramUniform3fv(prog, CrHash("u_camPos"), 1, camPos.v);
	app->shaderContext.matDiffuse = crVec4(1.0f, 1.0f, 1.0f, 1);
	app->shaderContext.matSpecular = crVec4(0.0f, 0.0f, 0.0f, 1);
	app->shaderContext.matShininess = 64;

	// draw wall
	{ CrVec3 v = {0, 1.0f, -2.5f};
	CrMat44 m;
	crMat44SetIdentity(&m);
	crMat44SetTranslation(&m, &v);
	
	app->shaderContext.worldMtx = m;
	crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
	crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m); }

	appShaderContextPreRender(app, sceneMtl);

	meshPreRender(floorMesh, prog);
	meshRenderTriangles(floorMesh);
}

void drawWater(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx, CrVec3 camPos)
{
	CrGpuState* gpuState = &crContext()->gpuState;
	CrGpuProgram* prog = waterMtl->program;
	
	gpuState->cull = CrFalse;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(waterMtl->program);

	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_water"), water->buffers[WaterBuffer_Position0 + water->curr], &sampler);}
	
	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_refract"), refractTex, &sampler);}

	//{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Wrap, CrSamplerAddress_Wrap};
	//crGpuProgramUniformTexture(prog, CrHash("u_water"), texture, &sampler); }

	{ float val[] = {16.0f / refractTex->width, 16.0f / refractTex->height, 0, 0};
	crGpuProgramUniform4fv(prog, CrHash("u_refractionMapParam"), 1, val);}

	crGpuProgramUniform3fv(prog, CrHash("u_camPos"), 1, camPos.v);

	// draw water plane
	app->shaderContext.matDiffuse = crVec4(0.75f, 1.0f, 0.75f, 1);
	app->shaderContext.matSpecular = crVec4(1, 1, 1, 1);
	app->shaderContext.matShininess = 64;
	{ CrMat44 m;
	crMat44MakeRotation(&m, CrVec3_c100(), -90);
	
	app->shaderContext.worldMtx = m;
	crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
	crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
	}
	appShaderContextPreRender(app, waterMtl);

	meshPreRender(waterMesh, prog);
	meshRenderTriangles(waterMesh);
}

void crMat44Unproject(CrVec3* _out, const CrVec3* screenPos, const CrMat44* m)
{
	CrVec4 v = {screenPos->x, screenPos->y, screenPos->z, 1};

	v.x = 2 * v.x - 1;
	v.y = 1 - 2 * v.y;

	crMat44Transform(&v, m);

	{float invw = 1.0f / v.w;
	_out->x = v.x * invw;
	_out->y = v.y * invw;
	_out->z = v.z * invw;}
}

CrBool crRayPlaneIntersect(float* t, const CrVec3* rayOrig, const CrVec3* rayDir, const CrVec3* planeN, float planeD)
{
	float det = crVec3Dot(rayDir, planeN);

	if(fabsf(det) < 1e-5f)
		return CrFalse;

	*t = -(crVec3Dot(rayOrig, planeN) + planeD) / det;
	return CrTrue;
}

void addDrop(CrMat44 viewProjMtx)
{
	float r = 8.0f / water->size;
	float s = 1 / 64.0f;
	float x = (float)input.x;
	float y = (float)input.y;

	CrMat44 invVPMtx;
	CrVec3 orgSs, endSs, orgWs, endWs, dirWs;
	CrVec3 waterN = {0, 1, 0};
	float waterD = 0;
	float t;
	
	if(CrFalse == crMat44Inverse(&invVPMtx, &viewProjMtx))
		return;
	
	x -= (float)0;	// viewport x
	y -= (float)0;	// viewport y

	x /= (float)crContext()->xres; // viewport.w
	y /= (float)crContext()->yres; // viewport.h

	orgSs = crVec3(x, y, 0);
	endSs = crVec3(x, y, 1);

	crMat44Unproject(&orgWs, &orgSs, &invVPMtx);
	crMat44Unproject(&endWs, &endSs, &invVPMtx);

	crVec3Sub(&dirWs, &endWs, &orgWs);

	if(CrFalse == crRayPlaneIntersect(&t, &orgWs, &dirWs, &waterN, waterD))
		return;

	// convert texture space
	x =  (orgWs.x + dirWs.x * t);
	y = -(orgWs.z + dirWs.z * t);
	
	x = (x / waterSize) * 0.5f + 0.5f;
	y = (y / waterSize) * 0.5f + 0.5f;
	waterAddDrop(water, x, y, r, s);
}

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	//remoteConfigLock(config);
	lsettings = settings;
	//remoteConfigUnlock(config);
}

void crAppHandleMouse(int x, int y, int action)
{
	if(CrApp_MouseDown == action) {
		input.x = x;
		input.y = y;
		input.isDown = CrTrue;
	}
	else if(CrApp_MouseUp == action) {
		input.x = x;
		input.y = y;
		input.isDown = CrFalse;
	}
	else if((CrApp_MouseMove == action) && (CrTrue == input.isDown)) {
		input.x = x;
		input.y = y;
	}
}

void crAppRender()
{
	//CrVec3 eyeAt = crVec3(0, 1.5f, 2);
	CrVec3 eyeAt = crVec3(0, 2.0f, 4.0f);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 60.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	// update water
	if(CrFalse == water->inited) {
		waterInit(water);
	}
	if(CrTrue == input.isDown) {
		addDrop(viewProjMtx);
	}

	waterStep(water);

	// render to refractTex
	{ CrTexture* bufs[] = {refractTex, nullptr};
	crContextPreRTT(crContext(), bufs, rttDepth);
	}
	crContextSetViewport(crContext(), 0, 0, (float)refractTex->width, (float)refractTex->height, -1, 1);

	crContextClearDepth(crContext(), 1);
	drawBackground();
	{ CrMat44 r, v, vp;
	crMat44PlanarReflect(&r, &waterN, &waterP);
	crMat44Mult(&v, &viewMtx, &r);
	crMat44Mult(&vp, &projMtx, &v);
	drawScene(v, projMtx, vp, eyeAt);
	}

	crContextPostRTT(crContext());
	crContextSetViewport(crContext(), 0, 0, (float)crContext()->xres, (float)crContext()->yres, -1, 1);

	// render to screen
	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene(viewMtx, projMtx, viewProjMtx, eyeAt);

	drawWater(viewMtx, projMtx, viewProjMtx, eyeAt);
}

void crAppConfig()
{
	crAppContext.appName = "Water";
	crAppContext.context->xres = 480;
	crAppContext.context->yres = 854;
}

void crAppFinalize()
{
	//remoteConfigFree(config);
	waterFree(water);
	meshFree(floorMesh);
	meshFree(waterMesh);
	meshFree(bgMesh);
	materialFree(waterMtl);
	materialFree(sceneMtl);
	materialFree(bgMtl);
	crTextureFree(texture);
	crTextureFree(refractTex);
	crTextureFree(rttDepth);
	appFree(app);
}

CrBool crAppInitialize()
{
	app = appAlloc();
	appInit(app);
	
	// remote config
	/*
	{ RemoteVarDesc descs[] = {
		{"shadowSlopScale", &settings.shadowSlopScale, 0, 8},
		{nullptr, nullptr, 0, 0}
	};
	
	config = remoteConfigAlloc();
	remoteConfigInit(config, 8080, CrTrue);
	remoteConfigAddVars(config, descs);
	}
	*/

	// water
	water = waterNew(256);

	// materials
	{ const char* directives[]  = {nullptr};
		
	appLoadMaterialBegin(app, directives);

	sceneMtl = appLoadMaterial(
		"Water.Scene.Vertex",
		"Water.Scene.Fragment",
		nullptr, nullptr, nullptr);

	waterMtl = appLoadMaterial(
		"Water.SceneWater.Vertex",
		"Water.SceneWater.Fragment",
		nullptr, nullptr, nullptr);
	
	bgMtl = appLoadMaterial(
		"Common.Bg.Vertex.20",
		"Common.Bg.Fragment.20",
		nullptr, nullptr, nullptr);
	
	appLoadMaterialEnd(app);
	}

	// textures
	texture = Pvr_createTexture(red_tile_texture);

	crDbgStr("create scene color buffers\n");
	refractTex = crTextureAlloc();
	crTextureInitRtt(refractTex, 512, 512, 0, 1, CrGpuFormat_UnormR8G8B8A8);
	
	crDbgStr("create scene depth buffers\n");
	rttDepth = crTextureAlloc();
	crTextureInitRtt(rttDepth, 512, 512, 0, 1, CrGpuFormat_Depth16);
	
	// floor
	{ CrVec3 offset = crVec3(-1.0f, -1.0f, 0);
	CrVec2 uvs = crVec2(1.0f, 1.0f);
	floorMesh = meshAlloc();
	meshInitWithQuad(floorMesh, 2, 2, &offset, &uvs, 1);
	}

	// water
	{ CrVec3 offset = crVec3(-waterSize, -waterSize, 0);
	CrVec2 uvs = crVec2(1.0f, 1.0f);
	waterMesh = meshAlloc();
	meshInitWithQuad(waterMesh, waterSize*2, waterSize*2, &offset, &uvs, 1);
	}

	// bg
	{ bgMesh = meshAlloc();
	meshInitWithScreenQuad(bgMesh);
	}

	crDbgStr("Water example started\n");

	return CrTrue;
}
