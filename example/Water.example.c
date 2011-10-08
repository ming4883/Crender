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

#define BallCount 2
Sphere ball[BallCount];
Mesh* ballMesh = nullptr;

Mesh* floorMesh = nullptr;
Mesh* waterMesh = nullptr;
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* bgMtl = nullptr;
CrTexture* texture = nullptr;

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

typedef struct Mouse
{
	CrBool isDown;
	int x;
	int y;
	CrVec3 clothOffsets[2];
	
} Mouse;

Mouse mouse = {0};

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
	WaterMaterial_Normal,
	WaterMaterial_AddDrop,
	WaterMaterial_Count,

} WaterMaterial;

/*
sceneMtl = appLoadMaterial(
			"Water.Scene.Vertex",
			"Water.Scene.Fragment",
			nullptr, nullptr, nullptr);
*/

typedef struct Water
{
	size_t size;
	size_t currentBuffer;
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
	for(i=0; i<WaterBuffer_Count; ++i) {
		self->buffers[i] = crTextureAlloc();
		crTextureInitRtt(self->buffers[i], size, size, 0, 1, CrGpuFormat_FloatR16G16B16A16);
	}

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

void waterSwapBuffers(Water* self)
{
	CrTexture* tmp = self->buffers[WaterBuffer_Position0];
	self->buffers[WaterBuffer_Position0] = self->buffers[WaterBuffer_Position1];
	self->buffers[WaterBuffer_Position1] = tmp;
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

	self->inited = CrTrue;
}

void waterStep(Water* self)
{
	size_t curr = WaterBuffer_Position0;
	size_t last = WaterBuffer_Position1;

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
	size_t curr = WaterBuffer_Position0;
	size_t last = WaterBuffer_Position1;

	CrGpuProgram* prog = self->materials[WaterMaterial_Normal]->program;

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
	size_t curr = WaterBuffer_Position0;
	size_t last = WaterBuffer_Position1;

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

	// draw balls
	{	
		int i;
		app->shaderContext.matDiffuse = crVec4(0.9f, 0.64f, 0.35f, 1);
		app->shaderContext.matSpecular = crVec4(1, 1, 1, 1);
		app->shaderContext.matShininess = 32;

		for(i=0; i<BallCount; ++i) {
			CrMat44 m;
			CrVec3 scale = {ball[i].radius, ball[i].radius, ball[i].radius};
			crMat44MakeScale(&m, &scale);
			crMat44SetTranslation(&m, &ball[i].center);
			
			app->shaderContext.worldMtx = m;
			crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);

			appShaderContextPreRender(app, sceneMtl);
			
			meshPreRender(ballMesh, sceneMtl->program);
			meshRenderTriangles(ballMesh);
		}
	}
}

void drawWater(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx)
{
	CrGpuState* gpuState = &crContext()->gpuState;
	
	gpuState->cull = CrFalse;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(sceneMtl->program);
	{	
		CrSampler sampler = {
			CrSamplerFilter_MagMin_Linear_Mip_None, 
			CrSamplerAddress_Wrap, 
			CrSamplerAddress_Wrap
		};
		crGpuProgramUniformTexture(sceneMtl->program, CrHash("u_tex"), water->buffers[WaterBuffer_Position0], &sampler);
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

		meshPreRender(waterMesh, sceneMtl->program);
		meshRenderTriangles(waterMesh);
	}
}

float dt = 0;
float w = 0.75f;

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	float elapsed = (float)elapsedMilliseconds / 1000;
	dt = (dt * w) + (elapsed * (1-w)); // low pass filter

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);

	refractionMapParam.z = settings.shadowSlopScale;

	t += dt * 0.1f * lsettings.impact;

	ball[0].center.z = cosf(t) * 3.f;
	ball[1].center.z = sinf(t) * 3.f;
}

void crAppHandleMouse(int x, int y, int action)
{
	if(CrApp_MouseDown == action) {
		mouse.x = x;
		mouse.y = y;
		mouse.isDown = CrTrue;
	}
	else if(CrApp_MouseUp == action) {
		mouse.isDown = CrFalse;
	}
	else if((CrApp_MouseMove == action) && (CrTrue == mouse.isDown)) {
		int dx = x - mouse.x;
		int dy = y - mouse.y;
		
		float mouseSensitivity = 0.0025f;
	}
}

void crAppRender()
{
	CrVec3 eyeAt = crVec3(-2.5f, 1.5f, 4);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene(viewMtx, projMtx, viewProjMtx);

	if(CrFalse == water->inited) {
		size_t i;
		waterInit(water);
		for(i=0; i<16; ++i) {
			float r = 12.0f / water->size;
			float x = (rand() % 0xffff) / ((float)0xffff);
			float y = (rand() % 0xffff) / ((float)0xffff);
			float s = 1 / 64.0f;
			waterAddDrop(water, x, y, r, s);
		}
	}
	waterStep(water);
	waterNormal(water);

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
	meshFree(ballMesh);
	meshFree(floorMesh);
	meshFree(waterMesh);
	meshFree(bgMesh);
	materialFree(sceneMtl);
	materialFree(bgMtl);
	crTextureFree(texture);
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
		
		bgMtl = appLoadMaterial(
			"Common.Bg.Vertex.20",
			"Common.Bg.Fragment.20",
			nullptr, nullptr, nullptr);
		
		appLoadMaterialEnd(app);
	}

	// textures
	{
		texture = Pvr_createTexture(red_tile_texture);
	}

	// balls
	{
		ball[0].center = crVec3(-0.5f, 0.5f, 0);
		ball[0].radius = 0.25f;
		ball[1].center = crVec3(0.5f, 0.5f, 0);
		ball[1].radius = 0.25f;
		ballMesh = meshAlloc();
		meshInitWithUnitSphere(ballMesh, 32);
	}

	// floor
	{
		CrVec3 offset = crVec3(-2.5f, -2.5f, 0);
		floorMesh = meshAlloc();
		meshInitWithQuad(floorMesh, 5, 5, &offset, 1);
	}

	// water
	{
		CrVec3 offset = crVec3(-2.5f, -2.5f, 0.5f);
		waterMesh = meshAlloc();
		meshInitWithQuad(waterMesh, 5, 5, &offset, 1);
	}

	// bg
	{
		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}

	water = waterNew(256);

	return CrTrue;
}
