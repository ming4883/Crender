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
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* bgMtl = nullptr;
CrTexture* texture = nullptr;
CrMat44 textureMtx;

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

	{	CrMat44 m; crMat44Transpose(&m, &textureMtx);
		crGpuProgramUniformMtx4fv(prog, CrHash("u_textureMtx"), 1, CrFalse, m.v);
	}
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

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	//remoteConfigLock(config);
	lsettings = settings;
	//remoteConfigUnlock(config);

	crMat44SetIdentity(&textureMtx);
	textureMtx.m01 = 1/16.0f * cosf(t);

	t += elapsedMilliseconds / 1000.0f;
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
	CrVec3 eyeAt = crVec3(0, 1.0f, 3.5f);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 60.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	// render to screen
	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene(viewMtx, projMtx, viewProjMtx, eyeAt);
}

void crAppConfig()
{
	crAppContext.appName = "TextureAnimation";
	crAppContext.context->xres = 480;
	crAppContext.context->yres = 854;
}

void crAppFinalize()
{
	//remoteConfigFree(config);
	meshFree(floorMesh);
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
		"TextureAnimation.Scene.Vertex",
		"TextureAnimation.Scene.Fragment",
		nullptr, nullptr, nullptr);
	
	bgMtl = appLoadMaterial(
		"Common.Bg.Vertex.20",
		"Common.Bg.Fragment.20",
		nullptr, nullptr, nullptr);
	
	appLoadMaterialEnd(app);
	}

	// textures
	texture = Pvr_createTexture(red_tile_texture);

	// floor
	{ CrVec3 offset = crVec3(-1.0f, -1.0f, 0);
	CrVec2 uvs = crVec2(1.0f, 1.0f);
	floorMesh = meshAlloc();
	meshInitWithQuad(floorMesh, 2, 2, &offset, &uvs, 1);
	}

	// bg
	{ bgMesh = meshAlloc();
	meshInitWithScreenQuad(bgMesh);
	}

	crDbgStr("TextureAnimation example started\n");

	return CrTrue;
}
