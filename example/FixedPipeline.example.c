#include "Common.h"
#include "Remote.h"
#include "Mesh.h"
#include "Pvr.h"
#include "red_tile_texture.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;
CrVec4 bgClr = {1, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
CrTexture* texture = nullptr;

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

	CrGpuStateDesc* gpuState = &app->gpuState->desc;

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

			memcpy(gpuState->fixedTransformProj, &app->shaderContext.worldViewProjMtx, sizeof(CrMat44));
		}

		gpuState->depthTest = CrTrue;
		gpuState->cull = CrTrue;
		gpuState->fixedTexStage[0].opRGB = CrGpuState_FixedTexOp_Modulate;
		gpuState->fixedTexStage[0].argRGB0 = CrGpuState_FixedTexArg_Constant;
		gpuState->fixedTexStage[0].argRGB1 = CrGpuState_FixedTexArg_Texture;
		gpuState->fixedTexStage[0].opA = CrGpuState_FixedTexOp_Arg0;
		gpuState->fixedTexStage[0].argA0 = CrGpuState_FixedTexArg_Constant;
		
		gpuState->fixedTexConstant[0] = 0.5f;
		gpuState->fixedTexConstant[1] = 1.0f;
		gpuState->fixedTexConstant[2] = 1.0f;
		gpuState->fixedTexConstant[3] = 0.5f;

		crGpuStatePreRender(app->gpuState);

		{
			CrSampler sampler = {
				CrSamplerFilter_MagMinMip_Linear, 
				CrSamplerAddress_Wrap, 
				CrSamplerAddress_Wrap
			};
			crGpuBindFixedTexture(0, texture, &sampler);
		}

		meshPreRender(mesh, nullptr);
		meshRenderTriangles(mesh);
	}
}

void crAppConfig()
{
	crAppContext.appName = "FixedPipeline";

	crAppContext.context->apiMajorVer = 1;
}

void crAppFinalize()
{
	crTextureFree(texture);
	meshFree(mesh);
	remoteConfigFree(config);
	appFree(app);
}

CrBool crAppInitialize()
{
	app = appAlloc();
	appInit(app);

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

	// load texture
	{
		texture = Pvr_createTexture(red_tile_texture);
	}

	bgClr = crVec4(0.25f, 1, 0.25f, 1);

	crDbgStr("CRender FixedPipeline example started");

	return CrTrue;
}
