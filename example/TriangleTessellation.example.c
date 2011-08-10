#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Remote.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;

Mesh* tessMesh = nullptr;
Mesh* bgMesh = nullptr;
Material* tessMtl = nullptr;
Material* bgMtl = nullptr;

typedef struct Settings
{
	float tessLevel;
	float linearity;
} Settings;

Settings settings;

void drawBackground()
{
	static const CrVec4 c[] = {
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.145f, 0.31f, 0.405f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
		{0.57f, 0.85f, 1.0f, 1.0f},
	};

	CrGpuStateDesc* gpuState = &app->gpuState->desc;
	
	gpuState->depthTest = CrFalse;
	gpuState->cull = CrTrue;
	crGpuStatePreRender(app->gpuState);

	crGpuProgramPreRender(bgMtl->program);
	crGpuProgramUniform4fv(bgMtl->program, CrHash("u_colors"), 4, (const float*)c);

	meshPreRender(bgMesh, bgMtl->program);
	meshRenderTriangles(bgMesh);
}

void drawScene(Settings* settings)
{
	CrVec3 eyeAt = crVec3(-2.5f, 1.5f, 5);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;
	CrGpuStateDesc* gpuState = &app->gpuState->desc;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

	gpuState->depthTest = CrTrue;
	gpuState->cull = CrTrue;
	//gpuState->polygonMode = CrGpuState_PolygonMode_Line;
	crGpuStatePreRender(app->gpuState);

	crGpuProgramPreRender(tessMtl->program);
	
	// draw floor
	app->shaderContext.matDiffuse = crVec4(1.0f, 0.88f, 0.33f, 1);
	app->shaderContext.matSpecular = crVec4(2, 2, 2, 1);
	app->shaderContext.matShininess = 32;

	{
		CrMat44 m;
		CrVec3 axis = {1, 0, 0};
		crMat44MakeRotation(&m, &axis, -90);
		
		crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
		crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
	}

	appShaderContextPreRender(app, tessMtl);

	crGpuProgramUniform1fv(tessMtl->program, CrHash("u_tessLevel"), 1, (const float*)&(settings->tessLevel));
	crGpuProgramUniform1fv(tessMtl->program, CrHash("u_linearity"), 1, (const float*)&(settings->linearity));

	meshPreRender(tessMesh, tessMtl->program);
	meshRenderPatches(tessMesh);

	gpuState->polygonMode = CrGpuState_PolygonMode_Fill;
}

void crAppUpdate(unsigned int elapsedMilliseconds)
{
}

void crAppHandleMouse(int x, int y, int action)
{
}

void crAppRender()
{
	Settings localSettings;

	remoteConfigLock(config);
	localSettings = settings;
	localSettings.linearity /= 10.0f;
	remoteConfigUnlock(config);

	crRenderTargetClearDepth(1);

	drawBackground();
	drawScene(&localSettings);
}

void crAppConfig()
{
	crAppContext.appName = "Triangle Tessellation";
	crAppContext.context->apiMajorVer = 4;
	crAppContext.context->apiMinorVer = 1;
}

void crAppFinalize()
{
	meshFree(tessMesh);
	meshFree(bgMesh);
	materialFree(tessMtl);
	materialFree(bgMtl);
	remoteConfigFree(config);
	appFree(app);
}

CrBool crAppInitialize()
{
	app = appAlloc();
	appInit(app);

	settings.tessLevel = 8;
	settings.linearity = 3;

	// remote config
	{
		RemoteVarDesc descs[] = {
			{"tessLevel", &settings.tessLevel, 0, 16},
			{"linearity", &settings.linearity, 0, 10},
			{nullptr, nullptr, 0, 0},
		};

		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, CrTrue);
		remoteConfigAddVars(config, descs);
	}

	// materials
	{
		const char* directives[]  = {nullptr};
		
		appLoadMaterialBegin(app, directives);

		tessMtl = appLoadMaterial(
			"TriangleTessellation.Scene.Vertex",
			"TriangleTessellation.Scene.Fragment",
			"TriangleTessellation.Scene.TessControl",
			"TriangleTessellation.Scene.TessEvaluation",
			nullptr);
		
		bgMtl = appLoadMaterial(
			"Common.Bg.Vertex",
			"Common.Bg.Fragment",
			nullptr, nullptr, nullptr);
		
		appLoadMaterialEnd(app);
	}

	// meshs
	{
		tessMesh = meshAlloc();
		meshInitWithObjFile(tessMesh, "monkey.obj", app->istream);

		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}

	return CrTrue;
}
