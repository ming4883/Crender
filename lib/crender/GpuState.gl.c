#include "GpuState.gl.h"
#include "Mem.h"
#include "Context.h"
#include "Mat44.h"

CR_API CrGpuState* crGpuStateAlloc()
{
	CrGpuStateImpl* self = crMem()->alloc(sizeof(CrGpuStateImpl), "CrGpuState");
	memset(self, 0, sizeof(CrGpuStateImpl));
	return &self->i;
}

CR_API void crGpuStateFree(CrGpuState* self)
{
	if(nullptr == self)
		return;

	crMem()->free(self, "CrGpuState");
}

CR_API void crGpuStateInit(CrGpuState* self)
{
	size_t i;

	self->desc.depthTest = CrTrue;
	self->desc.depthWrite = CrTrue;
	self->desc.cull = CrTrue;
	self->desc.blend = CrFalse;
	self->desc.blendFactorSrcRGB = CrGpuState_BlendFactor_One;
	self->desc.blendFactorDestRGB = CrGpuState_BlendFactor_Zero;
	self->desc.blendFactorSrcA = CrGpuState_BlendFactor_One;
	self->desc.blendFactorDestA = CrGpuState_BlendFactor_Zero;
	self->desc.polygonMode = CrGpuState_PolygonMode_Fill;

	for(i=0; i<2; ++i) {
		struct CrGpuStateFixedTexDesc* stage = &self->desc.fixedTexStage[i];
		stage->opRGB = CrGpuState_FixedTexOp_Arg0;
		stage->argRGB0 = CrGpuState_FixedTexArg_Texture;
		stage->argRGB1 = CrGpuState_FixedTexArg_Texture;
		stage->argRGB2 = CrGpuState_FixedTexArg_Texture;

		stage->opA = CrGpuState_FixedTexOp_Arg0;
		stage->argA0 = CrGpuState_FixedTexArg_Texture;
		stage->argA1 = CrGpuState_FixedTexArg_Texture;
		stage->argA2 = CrGpuState_FixedTexArg_Texture;
	}

	crMat44SetIdentity((CrMat44*)self->desc.fixedTransformModel);
	crMat44SetIdentity((CrMat44*)self->desc.fixedTransformProj);

	self->flags = CrGpuState_Inited;
}

static GLenum CrGpuState_blendFactorMapping[] = {
	GL_ONE,
	GL_ZERO,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
};

#if !defined(CR_GLES_2)
static GLenum CrGpuState_polygonModeMapping[] = {
	GL_LINE,
	GL_FILL,
};
#endif

#ifndef CR_ANDROID

static GLenum CrGpuState_fixedTexOpMapping[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_ADD,
	GL_ADD_SIGNED,
	GL_INTERPOLATE,
	GL_SUBTRACT,
};

static GLenum CrGpuState_fixedTexArgMapping[] = {
	GL_TEXTURE,
	GL_CONSTANT,
	GL_PRIMARY_COLOR,
	GL_PREVIOUS,
};

#endif

CR_API void crGpuStatePreRender(CrGpuState* self)
{
	crCheckGLError();	// clear any unhandled gl errors

	if(self->desc.depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(self->desc.depthWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if(self->desc.cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if(self->desc.blend) {
		glEnable(GL_BLEND);

		if(crContextFixedPipelineOnly()) {
			glBlendFunc(
				CrGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - CrGpuState_BlendFactor_One]);
		}
		else{
			glBlendFuncSeparate(
				CrGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->desc.blendFactorSrcA - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->desc.blendFactorDestA - CrGpuState_BlendFactor_One]);
		}
	}
	else {
		glDisable(GL_BLEND);
	}

#if !defined(CR_GLES_2)
	glPolygonMode(GL_FRONT_AND_BACK, CrGpuState_polygonModeMapping[self->desc.polygonMode - CrGpuState_PolygonMode_Line]);
#endif

#ifndef CR_ANDROID
	if(crContextFixedPipelineOnly()) {
		CrMat44 m;
		size_t i;

		for(i=0; i<1; ++i) {
			struct CrGpuStateFixedTexDesc* stage = &self->desc.fixedTexStage[i];

			GLenum opRGB = CrGpuState_fixedTexOpMapping[stage->opRGB - CrGpuState_FixedTexOp_Arg0];
			GLenum argRGB0 = CrGpuState_fixedTexArgMapping[stage->argRGB0 - CrGpuState_FixedTexArg_Texture];
			GLenum argRGB1 = CrGpuState_fixedTexArgMapping[stage->argRGB1 - CrGpuState_FixedTexArg_Texture];
			GLenum argRGB2 = CrGpuState_fixedTexArgMapping[stage->argRGB2 - CrGpuState_FixedTexArg_Texture];

			GLenum opA = CrGpuState_fixedTexOpMapping[stage->opA - CrGpuState_FixedTexOp_Arg0];
			GLenum argA0 = CrGpuState_fixedTexArgMapping[stage->argA0 - CrGpuState_FixedTexArg_Texture];
			GLenum argA1 = CrGpuState_fixedTexArgMapping[stage->argA1 - CrGpuState_FixedTexArg_Texture];
			GLenum argA2 = CrGpuState_fixedTexArgMapping[stage->argA2 - CrGpuState_FixedTexArg_Texture];

			glActiveTexture(GL_TEXTURE0 + i);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, opRGB);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, argRGB0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, argRGB1);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, argRGB2);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, opA);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, argA0);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, argA1);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, argA2);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
		}

		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, self->desc.fixedTexConstant);

		glDisable(GL_LIGHTING);

		glMatrixMode(GL_MODELVIEW);
		crMat44Transpose(&m, (CrMat44*)self->desc.fixedTransformModel);
		glLoadMatrixf((float*)&m);

		glMatrixMode(GL_PROJECTION);
		crMat44Transpose(&m, (CrMat44*)self->desc.fixedTransformProj);
		glLoadMatrixf((float*)&m);
	}
#endif
}
