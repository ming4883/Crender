#ifndef __CRENDER_CONTEXT_H__
#define __CRENDER_CONTEXT_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CrGpuStateType
{
	// BlendFactor
	CrGpuState_BlendFactor_One,
	CrGpuState_BlendFactor_Zero,
	CrGpuState_BlendFactor_SrcColor,
	CrGpuState_BlendFactor_OneMinusSrcColor,
	CrGpuState_BlendFactor_DestColor,
	CrGpuState_BlendFactor_OneMinusDestColor,
	CrGpuState_BlendFactor_SrcAlpha,
	CrGpuState_BlendFactor_OneMinusSrcAlpha,
	CrGpuState_BlendFactor_DestAlpha,
	CrGpuState_BlendFactor_OneMinusDestAlpha,

	// PolygonMode
	CrGpuState_PolygonMode_Line,
	CrGpuState_PolygonMode_Fill,

} CrGpuStateType;

typedef struct CrGpuState
{
	CrBool depthTest;
	CrBool depthWrite;
	CrBool cull;
	CrBool blend;
	CrGpuStateType blendFactorSrcRGB;
	CrGpuStateType blendFactorDestRGB;
	CrGpuStateType blendFactorSrcA;
	CrGpuStateType blendFactorDestA;
	CrGpuStateType polygonMode;

} CrGpuState;

typedef enum CrFfpStateType
{
	// TexOp
	CrFfpState_TexOp_Arg0,
	CrFfpState_TexOp_Modulate,
	CrFfpState_TexOp_Add,
	CrFfpState_TexOp_AddSigned,
	CrFfpState_TexOp_Lerp,
	CrFfpState_TexOp_Subtract,

	// TexArg
	CrFfpState_TexArg_Texture,
	CrFfpState_TexArg_Constant,
	CrFfpState_TexArg_Color,
	CrFfpState_TexArg_Current,

} CrFfpStateType;

typedef struct CrFfpTexStage
{
	CrFfpStateType opRGB;
	CrFfpStateType argRGB0;
	CrFfpStateType argRGB1;
	CrFfpStateType argRGB2;
	CrFfpStateType opA;
	CrFfpStateType argA0;
	CrFfpStateType argA1;
	CrFfpStateType argA2;

} CrFfpTexStage;

#define CR_MAX_FFP_TEX_STAGE 2
typedef struct CrFfpState
{
	float transformModel[16];
	float transformProj[16];
	float transformTex[16];
	
	float texConstant[4];
	struct CrFfpTexStage texStage[CR_MAX_FFP_TEX_STAGE];

} CrFfpState;

typedef struct CrContext
{
	const char* apiName;
	size_t apiMajorVer;
	size_t apiMinorVer;
	size_t xres;
	size_t yres;
	size_t msaaLevel;
	CrBool vsync;
	
	struct CrGpuState gpuState;
	struct CrFfpState ffpState;

} CrContext;

CR_API CrContext* crContext();

CR_API CrBool crContextFixedPipelineOnly();

CR_API CrContext* crContextAlloc();

CR_API CrBool crContextInit(CrContext* self, void** window);

CR_API void crContextFree(CrContext* self);

CR_API CrBool crContextPreRender(CrContext* self);

CR_API void crContextPostRender(CrContext* self);

CR_API void crContextSwapBuffers(CrContext* self);

CR_API CrBool crContextChangeResolution(CrContext* self, size_t xres, size_t yres);

CR_API void crContextApplyGpuState(CrContext* self);

CR_API void crContextApplyFfpState(CrContext* self);

CR_API void crContextSetViewport(CrContext* self, float x, float y, float w, float h, float zmin, float zmax);

CR_API void crContextClearColor(CrContext* self, float r, float g, float b, float a);

CR_API void crContextClearDepth(CrContext* self, float z);

struct CrTexture;

CR_API CrBool crContextDepthTextureSupport();

CR_API CrBool crContextPreRTT(CrContext* self, struct CrTexture** colors, struct CrTexture* depth);

CR_API CrBool crContextPostRTT(CrContext* self);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_CONTEXT_H__
