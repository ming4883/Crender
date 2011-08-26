#include "Common.h"
#include "Mesh.h"
#include "Material.h"
#include "Label.h"

AppContext* app = nullptr;
Material* mtlText = nullptr;
Mesh* meshBg = nullptr;
Label* label = nullptr;
CrVec4 textColor = {1, 0, 0, 1};

void crAppUpdate(unsigned int elapsedMilliseconds)
{
}

void crAppHandleMouse(int x, int y, int action)
{
}

void crAppRender()
{
	CrGpuState* gpuState = &crContext()->gpuState;

	// render to texture
	crRenderTargetClearDepth(1);
	crRenderTargetClearColor(0.25f, 0.75f, 1.0f, 1.0f);

	// display the label
	gpuState->blend = CrTrue;
	gpuState->blendFactorSrcRGB = CrGpuState_BlendFactor_SrcAlpha;
	gpuState->blendFactorDestRGB = CrGpuState_BlendFactor_OneMinusSrcAlpha;
	gpuState->blendFactorSrcA = CrGpuState_BlendFactor_SrcAlpha;
	gpuState->blendFactorDestA = CrGpuState_BlendFactor_OneMinusSrcAlpha;
	crContextApplyGpuState(crContext());
	
	crGpuProgramPreRender(mtlText->program);
	{
		CrSampler sampler = {
			CrSamplerFilter_MagMin_Linear_Mip_None,
			CrSamplerAddress_Clamp,
			CrSamplerAddress_Clamp
		};
		crGpuProgramUniformTexture(mtlText->program, CrHash("u_tex"), label->texture, &sampler);
	}
	crGpuProgramUniform4fv(mtlText->program, CrHash("u_textColor"), 1, textColor.v);
	
	meshPreRender(meshBg, mtlText->program);
	meshRenderTriangles(meshBg);

	gpuState->blend = CrFalse;
}

void crAppConfig()
{
	crAppContext.appName = "Label";
	crAppContext.context->xres = 400;
	crAppContext.context->yres = 300;
	crAppContext.context->apiMajorVer = 3;
	crAppContext.context->apiMinorVer = 3;
}

void crAppFinalize()
{
	meshFree(meshBg);
	materialFree(mtlText);
	Label_free(label);
	appFree(app);
}

CrBool crAppInitialize()
{
	char utf8[] = {0xEF, 0xBB, 0xBF, 0xE9, 0x80, 0x99, 0xE6, 0x98, 0xAF, 0x55, 0x54, 0x46, 0x38, 0x00};

	app = appAlloc();
	appInit(app);
	
	// label
	{
		label = Label_alloc();
		Label_init(label, crAppContext.context->xres, crAppContext.context->yres);
		Label_setText(label, utf8);
		Label_commit(label);
	}

	// materials
	{
		const char* directives[] = {nullptr};
		appLoadMaterialBegin(app, directives);

		mtlText = appLoadMaterial(
			"Common.Ui.Vertex",
			"Common.Text.Fragment",
			nullptr, nullptr, nullptr);

		appLoadMaterialEnd(app);
	}

	// mesh
	{
		meshBg = meshAlloc();
		meshInitWithScreenQuad(meshBg);
	}

	return CrTrue;
}
