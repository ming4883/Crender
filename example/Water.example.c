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
RemoteConfig* config = nullptr;

Mesh* floorMesh = nullptr;
Mesh* waterMesh = nullptr;
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* waterMtl = nullptr;
Material* bgMtl = nullptr;
CrTexture* texture = nullptr;
CrTexture* refractTex = nullptr;
CrTexture* rttDepth = nullptr;

CrMat44 refractionMapMtx;
size_t refractionMapSize = 1024;
CrVec4 refractionMapParam = { 1.f / 1024, 1e-3f * 1.5f, 0, 0};

typedef struct Settings
{
	float gravity;
	float airResistance;
	float impact;
	float shadowSlopScale;
} Settings;

Settings settings = {10, 5, 3, 4};

typedef struct Input
{
	CrBool isDown;
	int x;
	int y;
	CrBool addDrop;
	
} Input;

Input input = {0};

typedef enum WaterBuffer
{
	WaterBuffer_Position0,
	WaterBuffer_Position1,
	WaterBuffer_Normal,
	WaterBuffer_Count,
} WaterBuffer;

typedef enum WaterMaterial
{
	WaterMaterial_Init,
	WaterMaterial_Step,
	WaterMaterial_Normal,
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

Water* waterNew(size_t size)
{
	size_t i;
	Water* self = crMem()->alloc(sizeof(Water), "water");
	memset(self, 0, sizeof(Water));

	self->size = size;
	for(i=0; i<2; ++i) {
		size_t id = WaterBuffer_Position0 + i;
		self->buffers[id] = crTextureAlloc();
		crTextureInitRtt(self->buffers[id], size, size, 0, 1, CrGpuFormat_FloatR32G32B32A32);
	}

	self->buffers[WaterBuffer_Normal] = crTextureAlloc();
	crTextureInitRtt(self->buffers[WaterBuffer_Normal], size, size, 0, 1, CrGpuFormat_UnormR8G8B8A8);

	self->screenQuad = meshAlloc();
	meshInitWithScreenQuad(self->screenQuad);

	appLoadMaterialBegin(app, nullptr);

	self->materials[WaterMaterial_Init] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.Init.Fragment",
		nullptr, nullptr, nullptr);

	self->materials[WaterMaterial_Step] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.Step.Fragment",
		nullptr, nullptr, nullptr);

	self->materials[WaterMaterial_Normal] = appLoadMaterial(
		"Water.Process.Vertex",
		"Water.Normal.Fragment",
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

void waterNormal(Water* self)
{
	size_t curr = WaterBuffer_Position0 + waterCurrBuffer(self);
	
	CrGpuProgram* prog = self->materials[WaterMaterial_Normal]->program;

	waterPreProcess(self, self->buffers[WaterBuffer_Normal]);

	crGpuProgramPreRender(prog);
	{ float val[] = {1.0f / self->size, 1.0f / self->size};
	crGpuProgramUniform2fv(prog, CrHash("u_delta"), 1, val); }

	crGpuProgramUniformTexture(prog, CrHash("u_buffer"), self->buffers[curr], &self->psampler);

	meshPreRender(self->screenQuad, prog);
	meshRenderTriangles(self->screenQuad);

	waterPostProcess(self);
}

void waterAddDrop(Water* self, float x, float y, float r, float s)
{
	size_t curr = WaterBuffer_Position0 + waterCurrBuffer(self);
	size_t last = WaterBuffer_Position0 + waterLastBuffer(self);

	CrGpuProgram* prog = self->materials[WaterMaterial_AddDrop]->program;

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

void drawScene(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx)
{
	CrGpuState* gpuState = &crContext()->gpuState;

	gpuState->cull = CrTrue;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(sceneMtl->program);
	{	
		CrSampler sampler = {
			CrSamplerFilter_MagMinMip_Linear, 
			CrSamplerAddress_Wrap, 
			CrSamplerAddress_Wrap
		};
		crGpuProgramUniformTexture(sceneMtl->program, CrHash("u_tex"), texture, &sampler);
	}
	
	{
		CrMat44 refractionMapTexMtx = refractionMapMtx;
		crMat44AdjustToAPIProjectiveTexture(&refractionMapTexMtx);
		crMat44Transpose(&refractionMapTexMtx, &refractionMapTexMtx);
		crGpuProgramUniformMtx4fv(sceneMtl->program, CrHash("u_refractionMapTexMtx"), 1, CrFalse, refractionMapTexMtx.v);
		crGpuProgramUniform4fv(sceneMtl->program, CrHash("u_refractionMapParam"), 1, refractionMapParam.v);
	}

	// draw floor
	{
		app->shaderContext.matDiffuse = crVec4(1.0f, 1.0f, 1.0f, 1);
		app->shaderContext.matSpecular = crVec4(0, 0, 0, 1);
		app->shaderContext.matShininess = 32;
		{
			CrMat44 m;
			crMat44MakeRotation(&m, CrVec3_c100(), -90);
			
			app->shaderContext.worldMtx = m;
			crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		appShaderContextPreRender(app, sceneMtl);

		meshPreRender(floorMesh, sceneMtl->program);
		meshRenderTriangles(floorMesh);
	}
}

void drawWater(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx)
{
	CrGpuState* gpuState = &crContext()->gpuState;
	CrGpuProgram* prog = waterMtl->program;
	
	gpuState->cull = CrFalse;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(waterMtl->program);

	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_water"), water->buffers[WaterBuffer_Normal], &sampler);}
	
	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_refract"), refractTex, &sampler);}

	{ float val[] = {32.0f / refractTex->width, 32.0f / refractTex->height, 0, 0};
	crGpuProgramUniform4fv(prog, CrHash("u_refractionMapParam"), 1, val);}

	// draw water plane
	{
		app->shaderContext.matDiffuse = crVec4(1.0f, 1.0f, 1.0f, 1);
		app->shaderContext.matSpecular = crVec4(0, 0, 0, 1);
		app->shaderContext.matShininess = 32;
		{
			CrMat44 m;
			crMat44MakeRotation(&m, CrVec3_c100(), -90);
			
			app->shaderContext.worldMtx = m;
			crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		appShaderContextPreRender(app, waterMtl);

		meshPreRender(waterMesh, prog);
		meshRenderTriangles(waterMesh);
	}
}

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);

	refractionMapParam.z = settings.shadowSlopScale;

}

void crAppHandleMouse(int x, int y, int action)
{
	if(CrApp_MouseDown == action) {
		input.x = x;
		input.y = y;
		input.isDown = CrTrue;
	}
	else if(CrApp_MouseUp == action) {
		input.isDown = CrFalse;
		input.addDrop = CrTrue;
	}
	else if((CrApp_MouseMove == action) && (CrTrue == input.isDown)) {
		//int dx = x - input.x;
		//int dy = y - input.y;
		//float mouseSensitivity = 0.0025f;
	}
}

void crAppRender()
{
	CrVec3 eyeAt = crVec3(0, 1.5f, 3);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	// update water
	if(CrFalse == water->inited) {
		waterInit(water);
	}
	//if(CrTrue == input.addDrop) {
	{
		float r = 12.0f / water->size;
		float x = (abs(rand()) % 0xffff) / (float)0xffff;
		float y = (abs(rand()) % 0xffff) / (float)0xffff;
		float s = 1 / 64.0f;
		waterAddDrop(water, x, y, r, s);
		input.addDrop = CrFalse;
	}

	waterStep(water);
	waterNormal(water);

	// render to refractTex
	{ CrTexture* bufs[] = {refractTex, nullptr};
	crContextPreRTT(crContext(), bufs, rttDepth);}
	crContextSetViewport(crContext(), 0, 0, (float)refractTex->width, (float)refractTex->height, -1, 1);

	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene(viewMtx, projMtx, viewProjMtx);

	crContextPostRTT(crContext());
	crContextSetViewport(crContext(), 0, 0, (float)crContext()->xres, (float)crContext()->yres, -1, 1);

	// render to screen
	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene(viewMtx, projMtx, viewProjMtx);

	drawWater(viewMtx, projMtx, viewProjMtx);
}

void crAppConfig()
{
	crAppContext.appName = "Water";
	crAppContext.context->xres = 854;
	crAppContext.context->yres = 480;
}

void crAppFinalize()
{
	remoteConfigFree(config);
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
	{
		RemoteVarDesc descs[] = {
			{"gravity", &settings.gravity, 1, 100},
			{"airResistance", &settings.airResistance, 1, 20},
			{"impact", &settings.impact, 1, 10},
			{"shadowSlopScale", &settings.shadowSlopScale, 0, 8},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, CrTrue);
		remoteConfigAddVars(config, descs);
	}

	// materials
	{
		const char* directives[]  = {nullptr};
		
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
	{
		texture = Pvr_createTexture(red_tile_texture);

		refractTex = crTextureAlloc();
		crTextureInitRtt(refractTex, 512, 512, 0, 1, CrGpuFormat_UnormR8G8B8A8);
		
		rttDepth = crTextureAlloc();
		crTextureInitRtt(rttDepth, 512, 512, 0, 1, CrGpuFormat_Depth16);
	}

	// floor
	{
		CrVec3 offset = crVec3(-2.5f, -2.5f, 0);
		CrVec2 uvs = crVec2(5.0f, 5.0f);
		floorMesh = meshAlloc();
		meshInitWithQuad(floorMesh, 5, 5, &offset, &uvs, 1);
	}

	// water
	{
		CrVec3 offset = crVec3(-2.5f, -2.5f, 0.5f);
		CrVec2 uvs = crVec2(1.0f, 1.0f);
		waterMesh = meshAlloc();
		meshInitWithQuad(waterMesh, 5, 5, &offset, &uvs, 1);
	}

	// bg
	{
		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}

	water = waterNew(512);

	return CrTrue;
}
