#include "Context.gl.h"
#include "Mat44.h"

CR_API void crGpuStateInit(CrGpuState* self)
{
	self->depthTest = CrTrue;
	self->depthWrite = CrTrue;
	self->cull = CrTrue;
	self->blend = CrFalse;
	self->blendFactorSrcRGB = CrGpuState_BlendFactor_One;
	self->blendFactorDestRGB = CrGpuState_BlendFactor_Zero;
	self->blendFactorSrcA = CrGpuState_BlendFactor_One;
	self->blendFactorDestA = CrGpuState_BlendFactor_Zero;
	self->polygonMode = CrGpuState_PolygonMode_Fill;

}

CR_API void crFfpStateInit(CrFfpState* self)
{
	size_t i;
	for(i=0; i<CR_MAX_FFP_TEX_STAGE; ++i) {
		struct CrFfpTexStage* stage = &self->texStage[i];
		
		stage->opRGB = CrFfpState_TexOp_Arg0;
		stage->argRGB0 = CrFfpState_TexArg_Texture;
		stage->argRGB1 = CrFfpState_TexArg_Texture;
		stage->argRGB2 = CrFfpState_TexArg_Texture;

		stage->opA = CrFfpState_TexOp_Arg0;
		stage->argA0 = CrFfpState_TexArg_Texture;
		stage->argA1 = CrFfpState_TexArg_Texture;
		stage->argA2 = CrFfpState_TexArg_Texture;
	}

	memset(self->texConstant, 0, sizeof(self->texConstant));
	crMat44SetIdentity((CrMat44*)self->transformModel);
	crMat44SetIdentity((CrMat44*)self->transformProj);

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

static GLenum CrFfpState_TexOpMapping[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_ADD,
	GL_ADD_SIGNED,
	GL_INTERPOLATE,
	GL_SUBTRACT,
};

static GLenum CrFfpState_TexArgMapping[] = {
	GL_TEXTURE,
	GL_CONSTANT,
	GL_PRIMARY_COLOR,
	GL_PREVIOUS,
};

#endif

CR_API void crContextApplyGpuState(CrContext* self)
{
	crCheckGLError();	// clear any unhandled gl errors

	if(self->gpuState.depthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if(self->gpuState.depthWrite)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	if(self->gpuState.cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if(self->gpuState.blend) {
		glEnable(GL_BLEND);

		if(nullptr == glBlendFuncSeparate) {
			glBlendFunc(
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorSrcRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorDestRGB - CrGpuState_BlendFactor_One]);
		}
		else{
			glBlendFuncSeparate(
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorSrcRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorDestRGB - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorSrcA - CrGpuState_BlendFactor_One],
				CrGpuState_blendFactorMapping[self->gpuState.blendFactorDestA - CrGpuState_BlendFactor_One]);
		}
	}
	else {
		glDisable(GL_BLEND);
	}

#if !defined(CR_GLES_2)
	glPolygonMode(GL_FRONT_AND_BACK, CrGpuState_polygonModeMapping[self->gpuState.polygonMode - CrGpuState_PolygonMode_Line]);
#endif

}

CR_API void crContextApplyFfpState(CrContext* self)
{
	CrMat44 m;
	size_t i;
	
	crCheckGLError();	// clear any unhandled gl errors
	
	if(!crContextFixedPipelineOnly())
		return;

#ifndef CR_ANDROID
	for(i=0; i<1; ++i) {

		struct CrFfpTexStage* stage = &self->ffpState.texStage[i];

		GLenum opRGB = CrFfpState_TexOpMapping[stage->opRGB - CrFfpState_TexOp_Arg0];
		GLenum argRGB0 = CrFfpState_TexArgMapping[stage->argRGB0 - CrFfpState_TexArg_Texture];
		GLenum argRGB1 = CrFfpState_TexArgMapping[stage->argRGB1 - CrFfpState_TexArg_Texture];
		GLenum argRGB2 = CrFfpState_TexArgMapping[stage->argRGB2 - CrFfpState_TexArg_Texture];

		GLenum opA = CrFfpState_TexOpMapping[stage->opA - CrFfpState_TexOp_Arg0];
		GLenum argA0 = CrFfpState_TexArgMapping[stage->argA0 - CrFfpState_TexArg_Texture];
		GLenum argA1 = CrFfpState_TexArgMapping[stage->argA1 - CrFfpState_TexArg_Texture];
		GLenum argA2 = CrFfpState_TexArgMapping[stage->argA2 - CrFfpState_TexArg_Texture];

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

	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, self->ffpState.texConstant);

	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	crMat44Transpose(&m, (CrMat44*)self->ffpState.transformModel);
	glLoadMatrixf((float*)&m);

	glMatrixMode(GL_PROJECTION);
	crMat44Transpose(&m, (CrMat44*)self->ffpState.transformProj);
	glLoadMatrixf((float*)&m);

#endif
}

CR_API void crContextSetViewport(CrContext* self, float x, float y, float w, float h, float zmin, float zmax)
{
	glEnable(GL_SCISSOR_TEST);
	glViewport((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
	glScissor((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
	glDepthRange(zmin, zmax);
}

CR_API void crContextClearColor(CrContext* self, float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

CR_API void crContextClearDepth(CrContext* self, float z)
{
	glClearDepth(z);
	glClear(GL_DEPTH_BUFFER_BIT);
}