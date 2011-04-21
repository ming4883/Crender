#include "GpuState.gl.h"
#include "Mem.h"

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
	self->desc.depthTest = CrTrue;
	self->desc.depthWrite = CrTrue;
	self->desc.cull = CrTrue;
	self->desc.blend = CrFalse;
	self->desc.blendFactorSrcRGB = CrGpuState_BlendFactor_One;
	self->desc.blendFactorDestRGB = CrGpuState_BlendFactor_Zero;
	self->desc.blendFactorSrcA = CrGpuState_BlendFactor_One;
	self->desc.blendFactorDestA = CrGpuState_BlendFactor_Zero;
	self->desc.polygonMode = CrGpuState_PolygonMode_Fill;

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

CR_API void crGpuStatePreRender(CrGpuState* self)
{
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
		
		glBlendFuncSeparate(
			CrGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - CrGpuState_BlendFactor_One],
			CrGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - CrGpuState_BlendFactor_One],
			CrGpuState_blendFactorMapping[self->desc.blendFactorSrcA - CrGpuState_BlendFactor_One],
			CrGpuState_blendFactorMapping[self->desc.blendFactorDestA - CrGpuState_BlendFactor_One]);
	}
	else {
		glDisable(GL_BLEND);
	}

#if !defined(CR_GLES_2)
	glPolygonMode(GL_FRONT_AND_BACK, CrGpuState_polygonModeMapping[self->desc.polygonMode - CrGpuState_PolygonMode_Line]);
#endif
}
