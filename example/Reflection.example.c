#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Pvr.h"
#include "red_tile_texture.h"
#include "water_normal_map.h"
#include "water_flow_map.h"

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
CrTexture* waterNormalMap = nullptr;
CrTexture* waterFlowMap = nullptr;
CrTexture* refractTex = nullptr;
CrTexture* rttDepth = nullptr;

typedef struct Settings
{
	float shadowSlopScale;
} Settings;

float elapsedTime = 0;
float deltaTime = 0;

Settings settings = {4};
const CrVec3 waterN = {0.0f, 1.0f, 0.0f};
const CrVec3 waterP = {0.0f, 0.0f, 0.0f};
const float waterSize = 5.0f;

typedef struct Input
{
	CrBool isDown;
	int x;
	int y;
	
} Input;

Input input = {0};

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
	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Wrap, CrSamplerAddress_Wrap};
	crGpuProgramUniformTexture(prog, CrHash("u_tex"), texture, &sampler);
	}

	crGpuProgramUniform3fv(prog, CrHash("u_camPos"), 1, camPos.v);

	// draw wall
	{ CrVec3 v = {0, 1.25f, -1.0f};
	CrMat44 m;
	crMat44SetIdentity(&m);
	crMat44MakeRotation(&m, CrVec3_c010(), elapsedTime * 10.0f);
	crMat44SetTranslation(&m, &v);
	
	app->shaderContext.worldMtx = m;
	crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
	crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
	}

	appShaderContextPreRender(app, sceneMtl);

	meshPreRender(floorMesh, prog);
	meshRenderTriangles(floorMesh);
}

#define CYCLE 0.15f
#define HALF_CYCLE CYCLE * 0.5f

void drawWater(CrMat44 viewMtx, CrMat44 projMtx, CrMat44 viewProjMtx, CrVec3 camPos)
{
	CrGpuState* gpuState = &crContext()->gpuState;
	Material* mtl = waterMtl;
	CrGpuProgram* prog = mtl->program;
	
	gpuState->cull = CrFalse;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(mtl->program);

	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_refract"), refractTex, &sampler);
	}

	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Wrap, CrSamplerAddress_Wrap};
	crGpuProgramUniformTexture(prog, CrHash("u_water"), waterNormalMap, &sampler);
	}
	
	{ CrSampler sampler = {CrSamplerFilter_MagMin_Linear_Mip_None,  CrSamplerAddress_Clamp, CrSamplerAddress_Clamp};
	crGpuProgramUniformTexture(prog, CrHash("u_flow"), waterFlowMap, &sampler);
	}

	{ float t = deltaTime * 0.05f;
	static float p0 = 0;
	static float p1 = HALF_CYCLE;
	float val[] = {16.0f / refractTex->width, HALF_CYCLE, 0, 0};
	p0 += t; if(p0 >= CYCLE) p0 = 0;
	p1 += t; if(p1 >= CYCLE) p1 = 0;
	val[2] = p0;
	val[3] = p1;

	crGpuProgramUniform4fv(prog, CrHash("u_refractionMapParam"), 1, val);
	}

	crGpuProgramUniform3fv(prog, CrHash("u_camPos"), 1, camPos.v);

	// draw water plane
	app->shaderContext.matDiffuse = crVec4(1.0f, 1.0f, 1.0f, 1);
	app->shaderContext.matSpecular = crVec4(1.0f, 1.0f, 1.0f, 1);
	app->shaderContext.matShininess = 64;
	{ CrMat44 m;
	crMat44MakeRotation(&m, CrVec3_c100(), -90);

	app->shaderContext.worldMtx = m;
	crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
	crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
	}
	appShaderContextPreRender(app, mtl);

	meshPreRender(waterMesh, prog);
	meshRenderTriangles(waterMesh);
}

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	Settings lsettings;

	deltaTime = elapsedMilliseconds / 1000.0f;
	elapsedTime += deltaTime;

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
	CrVec3 eyeAt = crVec3(0, 2.0f, 6.0f);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 60.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

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
	crAppContext.appName = "Reflection";
	crAppContext.context->xres = 480;
	crAppContext.context->yres = 854;
}

void crAppFinalize()
{
	//remoteConfigFree(config);
	meshFree(floorMesh);
	meshFree(waterMesh);
	meshFree(bgMesh);
	materialFree(waterMtl);
	materialFree(sceneMtl);
	materialFree(bgMtl);
	crTextureFree(texture);
	crTextureFree(waterNormalMap);
	crTextureFree(waterFlowMap);
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

	// materials
	{ const char* directives[]  = {nullptr};
	appLoadMaterialBegin(app, directives);

	sceneMtl = appLoadMaterial(
		"Reflection.Scene.Vertex",
		"Reflection.Scene.Fragment",
		nullptr, nullptr, nullptr);

	waterMtl = appLoadMaterial(
		"Reflection.SceneWater.Vertex",
		"Reflection.SceneWater.Fragment",
		nullptr, nullptr, nullptr);
	
	bgMtl = appLoadMaterial(
		"Common.Bg.Vertex.20",
		"Common.Bg.Fragment.20",
		nullptr, nullptr, nullptr);
	
	appLoadMaterialEnd(app);
	}

	// textures
	texture = Pvr_createTexture(red_tile_texture);
	waterNormalMap = Pvr_createTexture(water_normal_map);
	waterFlowMap = Pvr_createTexture(water_flow_map);

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
	bgMesh = meshAlloc();
	meshInitWithScreenQuad(bgMesh);

	crDbgStr("Reflection example started\n");

	return CrTrue;
}
