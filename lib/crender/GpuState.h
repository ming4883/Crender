#ifndef __CRENDER_GPUSTATE_H__
#define __CRENDER_GPUSTATE_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CrGpuStateFlag
{
	CrGpuState_Inited = 1 << 0,
} CrGpuStateFlag;

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

	// FixedTexOp
	CrGpuState_FixedTexOp_Arg0,
	CrGpuState_FixedTexOp_Multiply,
	CrGpuState_FixedTexOp_Add,
	CrGpuState_FixedTexOp_Subtract,

	// FixedTexArg
	CrGpuState_FixedTexArg_Current,
	CrGpuState_FixedTexArg_Texture,
	CrGpuState_FixedTexArg_Color,
	CrGpuState_FixedTexArg_Constant,

} CrGpuStateType;

typedef struct CrGpuStateFixedTexDesc
{
	CrGpuStateType op;
	CrGpuStateType argRGB0;
	CrGpuStateType argRGB1;
	CrGpuStateType argA0;
	CrGpuStateType argA1;
};

typedef struct CrGpuStateDesc
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

	struct CrGpuStateFixedTexDesc fixedTexStage[2];
	float fixedTexConstant[4];

} CrGpuStateDesc;

typedef struct CrGpuState
{
	size_t flags;	// combinations of CrGpuStateFlag
	CrGpuStateDesc desc;

} CrGpuState;

CR_API CrGpuState* crGpuStateAlloc();

CR_API void crGpuStateFree(CrGpuState* self);

CR_API void crGpuStateInit(CrGpuState* self);

CR_API void crGpuStatePreRender(CrGpuState* self);


#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_GPUSTATE_H__