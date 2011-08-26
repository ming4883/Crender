#include "Common.h"
#include "Remote.h"
#include "Mesh.h"
#include "Pvr.h"
#include "red_tile_texture.h"

AppContext* app = nullptr;
RemoteConfig* config = nullptr;
CrVec4 bgClr = {0.25f, 0.25f, 0.25f, 1};
Mesh* mesh = nullptr;
Mesh* batchMesh = nullptr;
Material* mtl = nullptr;
CrTexture* texture = nullptr;

#define CNT 5
#define BATCH_DRAW 0

typedef struct Settings
{
	float size;
} Settings;

Settings settings = {100};

float t = 0;

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	//crDbgStr("dt = %d", elapsedMilliseconds);
	t += elapsedMilliseconds * 0.0001f;
	if(t > 1.0f) {
		t = 0.0f;
	}
}

void crAppHandleMouse(int x, int y, int action)
{
}

void crAppRender()
{
	size_t row, col;
	CrVec3 eyeAt = crVec3(0, 1.5f, 20);
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

	// setup gpu state
	{
		gpuState->depthTest = CrTrue;
		gpuState->cull = CrTrue;
		crContextApplyGpuState(crContext());

		crGpuProgramPreRender(mtl->program);
		{	
			CrSampler sampler = {
				CrSamplerFilter_MagMinMip_Linear, 
				CrSamplerAddress_Wrap, 
				CrSamplerAddress_Wrap
			};
			crGpuProgramUniformTexture(mtl->program, CrHash("u_tex"), texture, &sampler);
		}
	}

	// draw scene
	{
		CrVec3 scale = {lsettings.size / 100.f, lsettings.size / 100.f, lsettings.size / 100.f};
	
		CrMat44 m, r, s;
		crMat44MakeScale(&s, &scale);
		crMat44MakeRotation(&r, CrVec3_c100(), 360 * t);
		crMat44Mult(&m, &r, &s);

		if(BATCH_DRAW) {
			
			app->shaderContext.worldViewProjMtx = viewProjMtx;
			appShaderContextPreRender(app, mtl);

			
			meshPreRender(batchMesh, mtl->program);
			meshRenderTriangles(batchMesh);
		}
		else {

			meshPreRender(mesh, mtl->program);

			for(row = 0; row < CNT; ++row) {
				for(col = 0; col < CNT; ++col) {

					CrVec3 pos = {col*2.0f-CNT, row*2.0f-CNT, 0};
					crMat44SetTranslation(&m, &pos);

					crMat44Mult(&app->shaderContext.worldViewProjMtx, &viewProjMtx, &m);
					appShaderContextPreRender(app, mtl);

					meshRenderTriangles(mesh);
				}
			}
		}
	}
}

void crAppConfig()
{
	crAppContext.appName = "MeshPerformance";

	if(strcmp(crAppContext.context->apiName, "gl") == 0) {
		crAppContext.context->apiMajorVer = 3;
		crAppContext.context->apiMinorVer = 3;
	}
}

void crAppFinalize()
{
	meshFree(batchMesh);
	meshFree(mesh);
	crTextureFree(texture);
	materialFree(mtl);
	remoteConfigFree(config);
	appFree(app);
}

CrBool crAppInitialize()
{
	app = appAlloc();
	appInit(app);

	
	// remote config
	crDbgStr("init remote config...");
	{
		RemoteVarDesc descs[] = {
			{"size", &settings.size, 1, 100},
			{nullptr, nullptr, 0, 0}
		};
		
		config = remoteConfigAlloc();
		remoteConfigInit(config, 8080, CrTrue);
		remoteConfigAddVars(config, descs);
	}
	/**/

	// load materials
	crDbgStr("loading materials...");
	{
		appLoadMaterialBegin(app, nullptr);

		mtl = appLoadMaterial(
			"MeshPerformance.Scene.Vertex",
			"MeshPerformance.Scene.Fragment",
			nullptr, nullptr, nullptr);
		if(0 == (mtl->flags & MaterialFlag_Inited))
			return CrFalse;
		
		appLoadMaterialEnd(app);
	}

	crDbgStr("creating textures...");
	texture = Pvr_createTexture(red_tile_texture);

	crDbgStr("loading meshes...");
	// load mesh
	{
		mesh = meshAlloc();
		if(!meshInitWithObjFile(mesh, "monkey.obj", app->istream))
			return CrFalse;

		crDbgStr("mesh vcnt=%d, icnt=%d\n", mesh->vertexCount, mesh->indexCount);
	}

	crDbgStr("creating batch meshes...");
	{
		size_t instCnt = CNT * CNT;
		batchMesh = meshAlloc();
		meshInit(batchMesh, mesh->vertexCount * instCnt, mesh->indexCount * instCnt);
	}

	{
		size_t row, col;
		size_t i;
		
		unsigned char* dstVert = batchMesh->vertex.buffer;
		unsigned char* srcVert = mesh->vertex.buffer;

		unsigned char* dstUV = batchMesh->texcoord[0].buffer;
		unsigned char* srcUV = mesh->texcoord[0].buffer;

		unsigned char* dstIdx = batchMesh->index.buffer;
		unsigned char* srcIdx = mesh->index.buffer;

		unsigned short idxOffset = 0;

		CrMat44 m;
		crMat44SetIdentity(&m);
		
		for(row = 0; row < CNT; ++row) {
			for(col = 0; col < CNT; ++col) {

				CrVec3 pos = {col*2.0f-CNT, row*2.0f-CNT, 0};
				crMat44SetTranslation(&m, &pos);

				crMat44TransformAffinePts((CrVec3*)dstVert, (CrVec3*)srcVert, mesh->vertexCount, &m);
				dstVert += mesh->vertex.sizeInBytes;

				memcpy(dstUV, srcUV, mesh->texcoord[0].sizeInBytes);
				dstUV += mesh->texcoord[0].sizeInBytes;

				memcpy(dstIdx, srcIdx, mesh->index.sizeInBytes);
				
				{
					unsigned short* dst = (unsigned short*)dstIdx;
					unsigned short* src = (unsigned short*)srcIdx;
					for(i = 0; i<mesh->indexCount; ++i) {
						*dst = *src + idxOffset;
						++dst;
						++src;
					}
				}
				idxOffset += mesh->vertexCount;
				dstIdx += mesh->index.sizeInBytes;
			}
		}

		meshCommit(batchMesh);
	}
	
	crDbgStr("CRender MeshPerformance example started, batch=%d\n", BATCH_DRAW);

	return CrTrue;
}
