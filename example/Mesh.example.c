#include "Common.h"
#include "Remote.h"
#include "Mesh.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;
CrVec4 bgClr = {1, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
Material* mtl = nullptr;

typedef struct Settings
{
	float size;
} Settings;

Settings settings = {100};

float t = 0;

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	t += elapsedMilliseconds * 0.0001f;
	if(t > 1.0f) {
		t = 0.0f;
	}
}

void crAppHandleMouse(int x, int y, int action)
{
	crDbgStr("handle mouse x=%d, y=%d, a=%d\n", x, y, action); 
}

void crAppRender()
{
	CrVec3 eyeAt = crVec3(-2.5f, 1.5f, 5);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;

	CrGpuState* gpuState = &crContext()->gpuState;

	Settings lsettings;

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);
		
	// clear
	{
		crRenderTargetClearColor(bgClr.x, bgClr.y, bgClr.z, bgClr.w);
		crRenderTargetClearDepth(1);
	}
	
	// draw scene
	{
		app->shaderContext.matDiffuse = crVec4(1.0f, 0.88f, 0.33f, 1);
		app->shaderContext.matSpecular = crVec4(2, 2, 2, 1);
		app->shaderContext.matShininess = 32;

		{
			CrVec3 axis = {1, 0, 0};
			CrVec3 scale = {lsettings.size / 100.f, lsettings.size / 100.f, lsettings.size / 100.f};
			CrMat44 r, s;
			crMat44MakeScale(&s, &scale);
			crMat44MakeRotation(&r, &axis, 360 * t);

			crMat44Mult(&app->shaderContext.worldMtx, &r, &s);
			crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &app->shaderContext.worldMtx);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &app->shaderContext.worldMtx);
		}

		gpuState->depthTest = CrTrue;
		gpuState->cull = CrTrue;
		crContextApplyGpuState(crContext());

		crGpuProgramPreRender(mtl->program);
		appShaderContextPreRender(app, mtl);

		meshPreRender(mesh, mtl->program);
		meshRenderTriangles(mesh);
	}
}

void crAppConfig()
{
	crAppContext.appName = "Mesh";

	if(strcmp(crAppContext.context->apiName, "gl") == 0) {
		crAppContext.context->apiMajorVer = 3;
		crAppContext.context->apiMinorVer = 3;
	}
}

void crAppFinalize()
{
	meshFree(mesh);
	materialFree(mtl);
	remoteConfigFree(config);
	appFree(app);
}

CrBool crAppInitialize()
{
	app = appAlloc();
	appInit(app);
	appResize(app, 320, 240);

	// remote config
	{
		RemoteVarDesc descs[] = {
			{"size", &settings.size, 1, 100},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, CrTrue);
		remoteConfigAddVars(config, descs);
	}
	
	// load mesh
	{
		mesh = meshAlloc();
		if(!meshInitWithObjFile(mesh, "monkey.obj", app->istream))
			return CrFalse;
	}
	
	// load materials
	{
		appLoadMaterialBegin(app, nullptr);

		mtl = appLoadMaterial(
			"Mesh.Scene.Vertex",
			"Mesh.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtl->flags & MaterialFlag_Inited))
			return CrFalse;
		
		appLoadMaterialEnd(app);
	}
	
	bgClr = crVec4(0.25f, 1, 0.25f, 1);
	
	crDbgStr("XpRender Mesh example started");

	return CrTrue;
}
