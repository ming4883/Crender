#include "Common.h"
#include "Remote.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"
#include "Pvr.h"
#include "red_tile_texture.h"

#include "../lib/crender/Texture.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;

Cloth* cloth = nullptr;

#define BallCount 2
Sphere ball[BallCount];
Mesh* ballMesh = nullptr;

Mesh* floorMesh = nullptr;
Mesh* bgMesh = nullptr;

Material* sceneMtl = nullptr;
Material* bgMtl = nullptr;
Material* shadowMapMtl = nullptr;
CrTexture* texture = nullptr;

CrMat44 shadowMapMtx;
CrTexture* shadowMap = nullptr;
CrTexture* shadowMapZ = nullptr;
size_t shadowMapSize = 1024;
CrVec4 shadowMapParam = { 1.f / 1024, 1e-3f * 1.5f, 0, 0};

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

void computeShadowMapMatrix(CrMat44* m, const CrVec3* minPt, const CrVec3* maxPt)
{	
	CrMat44 viewMtx;
	CrMat44 projMtx;
	{
		CrVec3 eyeAt;
		CrVec3 lookAt;
		CrVec3 eyeUp;
		
		eyeAt.x = (maxPt->x + minPt->x) / 2;
		eyeAt.y = maxPt->y;
		eyeAt.z = (maxPt->z + minPt->z) / 2;

		lookAt = eyeAt;
		lookAt.y = minPt->y;

		lookAt.z = minPt->z;

		eyeUp = *CrVec3_c001();

		crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	}

	{
		int i;
		CrVec3 vmax, vmin;
		CrVec3 c[8] = {
			{minPt->x, minPt->y, minPt->z},
			{maxPt->x, minPt->y, minPt->z},
			{minPt->x, maxPt->y, minPt->z},
			{maxPt->x, maxPt->y, minPt->z},
			{minPt->x, minPt->y, maxPt->z},
			{maxPt->x, minPt->y, maxPt->z},
			{minPt->x, maxPt->y, maxPt->z},
			{maxPt->x, maxPt->y, maxPt->z},
		};

		for(i=0; i<8; ++i) {
			CrVec3* pt = &c[i];
			crMat44TransformAffinePt(pt, &viewMtx);
			if(0 ==i) {
				vmax = *pt;
				vmin = *pt;
			}
			else {
				vmax.x = crMax(vmax.x, pt->x);
				vmax.y = crMax(vmax.y, pt->y);
				vmax.z = crMax(vmax.z, pt->z);

				vmin.x = crMin(vmin.x, pt->x);
				vmin.y = crMin(vmin.y, pt->y);
				vmin.z = crMin(vmin.z, pt->z);
			}
		}

		crMat44SetIdentity(&projMtx);
		projMtx.m00 = 2 / (vmax.x - vmin.x);
		projMtx.m11 = 2 / (vmax.y - vmin.y);
		projMtx.m22 =-2 / (vmax.z - vmin.z);
	}
	
	crMat44Mult(m, &projMtx, &viewMtx);
	crMat44AdjustToAPIDepthRange(m);
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

void drawShadowMap()
{
	CrGpuState* gpuState = &crContext()->gpuState;
	
	CrTexture* bufs[] = {shadowMap, nullptr};
	crContextPreRTT(crContext(), bufs, shadowMapZ);
	crContextSetViewport(crContext(), 0, 0, (float)shadowMapSize, (float)shadowMapSize, -1, 1);
	crContextClearColor(crContext(), 1, 1, 1, 1);
	crContextClearDepth(crContext(), 1);

	// compute shadow map matrix
	{
		CrVec3 minPt = {-4, -4, -4};
		CrVec3 maxPt = { 4,  4,  4};
		computeShadowMapMatrix(&shadowMapMtx, &minPt, &maxPt);
	}
	
	gpuState->cull = CrTrue;
	gpuState->depthTest = CrTrue;
	crContextApplyGpuState(crContext());

	crGpuProgramPreRender(shadowMapMtl->program);
	crGpuProgramUniform4fv(shadowMapMtl->program, CrHash("u_shadowMapParam"), 1, shadowMapParam.v);

	// draw floor
	{	
		{
			CrMat44 m;
			crMat44MakeRotation(&m, CrVec3_c100(), -90);
			
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &shadowMapMtx, &m);
		}
		appShaderContextPreRender(app, shadowMapMtl);

		meshPreRender(floorMesh, shadowMapMtl->program);
		meshRenderTriangles(floorMesh);
	}

	// draw cloth
	{	
		gpuState->cull = CrFalse;
		crContextApplyGpuState(crContext());
		{
			CrMat44 m;
			crMat44SetIdentity(&m);

			crMat44Mult(&app->shaderContext.worldViewProjMtx, &shadowMapMtx, &m);
		}
		appShaderContextPreRender(app, shadowMapMtl);

		meshPreRender(cloth->mesh, shadowMapMtl->program);
		meshRenderTriangles(cloth->mesh);
		
		gpuState->cull = CrTrue;
		crContextApplyGpuState(crContext());
	}

	// draw balls
	{	
		int i;
		for(i=0; i<BallCount; ++i) {
			CrMat44 m;
			CrVec3 scale = {ball[i].radius, ball[i].radius, ball[i].radius};
			crMat44MakeScale(&m, &scale);
			crMat44SetTranslation(&m, &ball[i].center);
			
			//crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &shadowMapMtx, &m);

			appShaderContextPreRender(app, shadowMapMtl);
			
			meshPreRender(ballMesh, shadowMapMtl->program);
			meshRenderTriangles(ballMesh);
		}
	}

	//crRenderTargetPreRender(nullptr, nullptr, nullptr);
	crContextPostRTT(crContext());
	crContextSetViewport(crContext(), 0, 0, (float)crContext()->xres, (float)crContext()->yres, -1, 1);
}

void drawScene()
{
	CrVec3 eyeAt = crVec3(-2.5f, 1.5f, 4);
	CrVec3 lookAt = crVec3(0, 0, 0);
	CrVec3 eyeUp = *CrVec3_c010();
	CrMat44 viewMtx;
	CrMat44 projMtx;
	CrMat44 viewProjMtx;

	CrGpuState* gpuState = &crContext()->gpuState;
	
	crMat44CameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	crMat44Prespective(&projMtx, 45.0f, app->aspect.width / app->aspect.height, 0.1f, 30.0f);
	crMat44AdjustToAPIDepthRange(&projMtx);
	crMat44Mult(&viewProjMtx, &projMtx, &viewMtx);

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
		CrSampler sampler = {
			CrSamplerFilter_MagMin_Linear_Mip_None, 
			CrSamplerAddress_Clamp, 
			CrSamplerAddress_Clamp
		};
		crGpuProgramUniformTexture(sceneMtl->program, CrHash("u_shadowMapTex"), shadowMap, &sampler);
	}
	{
		CrMat44 shadowMapTexMtx = shadowMapMtx;
		crMat44AdjustToAPIProjectiveTexture(&shadowMapTexMtx);
		crMat44Transpose(&shadowMapTexMtx, &shadowMapTexMtx);
		crGpuProgramUniformMtx4fv(sceneMtl->program, CrHash("u_shadowMapTexMtx"), 1, CrFalse, shadowMapTexMtx.v);
		crGpuProgramUniform4fv(sceneMtl->program, CrHash("u_shadowMapParam"), 1, shadowMapParam.v);
	}

	// draw floor
	{	
		//app->shaderContext.matDiffuse = crVec4(1.0f, 0.88f, 0.33f, 1);
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

	// draw cloth
	{	
		app->shaderContext.matDiffuse = crVec4(1.0f, 0.22f, 0.0f, 1);
		app->shaderContext.matSpecular = crVec4(0.125f, 0.125f, 0.125f, 1);
		app->shaderContext.matShininess = 32;

		gpuState->cull = CrFalse;
		crContextApplyGpuState(crContext());
		{
			CrMat44 m;
			crMat44SetIdentity(&m);

			app->shaderContext.worldMtx = m;
			crMat44Mult(&app->shaderContext.worldViewMtx, &viewMtx, &m);
			crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
		}
		appShaderContextPreRender(app, sceneMtl);

		meshPreRender(cloth->mesh, sceneMtl->program);
		meshRenderTriangles(cloth->mesh);
		
		gpuState->cull = CrTrue;
		crContextApplyGpuState(crContext());
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

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	static float t = 0;
	Settings lsettings;

	int iter;
	CrVec3 f;

	float dt = (float)elapsedMilliseconds / 1000;

	remoteConfigLock(config);
	lsettings = settings;
	remoteConfigUnlock(config);

	shadowMapParam.z = settings.shadowSlopScale;

	//t += 0.0005f * lsettings.impact;
	t += dt * 0.1f * lsettings.impact;

	ball[0].center.z = cosf(t) * 5.f;
	ball[1].center.z = sinf(t) * 5.f;

	//cloth->timeStep = 0.01f;	// fixed time step
	cloth->timeStep = dt * 2;	// fixed time step
	cloth->damping = lsettings.airResistance * 1e-3f;

	// perform relaxation
	for(iter = 0; iter < 5; ++iter)
	{
		CrVec3 floorN = {0, 1, 0};
		CrVec3 floorP = {0, 0, 0};

		int i;
		for(i=0; i<BallCount; ++i)
			Cloth_collideWithSphere(cloth, &ball[i]);

		Cloth_collideWithPlane(cloth, &floorN, &floorP);
		Cloth_satisfyConstraints(cloth);
	}
	
	f = crVec3(0, -lsettings.gravity * cloth->timeStep, 0);
	Cloth_addForceToAll(cloth, &f);

	Cloth_verletIntegration(cloth);

	Cloth_updateMesh(cloth);
}

void crAppHandleMouse(int x, int y, int action)
{
	if(CrApp_MouseDown == action) {
		mouse.x = x;
		mouse.y = y;

		mouse.clothOffsets[0] = cloth->fixPos[0];
		mouse.clothOffsets[1] = cloth->fixPos[cloth->segmentCount-1];

		mouse.isDown = CrTrue;
	}
	else if(CrApp_MouseUp == action) {
		mouse.isDown = CrFalse;
	}
	else if((CrApp_MouseMove == action) && (CrTrue == mouse.isDown)) {
		int dx = x - mouse.x;
		int dy = y - mouse.y;
		
		float mouseSensitivity = 0.0025f;
		cloth->fixPos[0].x = mouse.clothOffsets[0].x + dx * mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].x = mouse.clothOffsets[1].x + dx * mouseSensitivity;

		cloth->fixPos[0].y = mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].y = mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}

void crAppRender()
{
	drawShadowMap();

	crContextClearDepth(crContext(), 1);
	drawBackground();
	drawScene();
}

void crAppConfig()
{
	crAppContext.appName = "Cloth Simulation";

	if(strcmp(crAppContext.context->apiName, "gl") == 0) {
		crAppContext.context->apiMajorVer = 3;
		crAppContext.context->apiMinorVer = 3;
	}
}

void crAppFinalize()
{
	remoteConfigFree(config);
	Cloth_free(cloth);
	meshFree(ballMesh);
	meshFree(floorMesh);
	meshFree(bgMesh);
	materialFree(shadowMapMtl);
	materialFree(sceneMtl);
	materialFree(bgMtl);
	crTextureFree(texture);
	crTextureFree(shadowMap);
	crTextureFree(shadowMapZ);
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

	// shadow map
	shadowMap = crTextureAlloc();
	crTextureInitRtt(shadowMap, shadowMapSize, shadowMapSize, 0, 1, CrGpuFormat_FloatR16);

	shadowMapZ = crTextureAlloc();
	crTextureInitRtt(shadowMapZ,shadowMapSize, shadowMapSize, 0, 1, CrGpuFormat_Depth16);

	// materials
	{
		const char* directives[]  = {nullptr};
		
		appLoadMaterialBegin(app, directives);

		sceneMtl = appLoadMaterial(
			"ClothSimulation.Scene.Vertex",
			"ClothSimulation.Scene.Fragment",
			nullptr, nullptr, nullptr);
		
		bgMtl = appLoadMaterial(
			"Common.Bg.Vertex",
			"Common.Bg.Fragment",
			nullptr, nullptr, nullptr);

		shadowMapMtl = appLoadMaterial(
			"ShadowMap.Create.Vertex",
			"ShadowMap.Create.Fragment",
			nullptr, nullptr, nullptr);
		
		appLoadMaterialEnd(app);
	}

	// textures
	{
		texture = Pvr_createTexture(red_tile_texture);
	}

	// cloth
	{
		CrVec3 offset = crVec3(-1, 1.5f, 0);
		cloth = Cloth_new(2, 2, &offset, 32);
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
		CrVec2 uvs = crVec2(5.0f, 5.0f);
		floorMesh = meshAlloc();
		meshInitWithQuad(floorMesh, 5, 5, &offset, &uvs, 1);
	}

	// bg
	{
		bgMesh = meshAlloc();
		meshInitWithScreenQuad(bgMesh);
	}


	return CrTrue;
}
