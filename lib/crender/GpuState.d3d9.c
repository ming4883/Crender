#include "GpuState.d3d9.h"
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

static D3DBLEND CrGpuState_blendFactorMapping[] = {
	D3DBLEND_ONE,
	D3DBLEND_ZERO,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
};

static D3DFILLMODE CrGpuState_polygonModeMapping[] = {
	D3DFILL_WIREFRAME,
	D3DFILL_SOLID,
};

CR_API void crGpuStatePreRender(CrGpuState* self)
{
	IDirect3DDevice9* d3ddev = crContextImpl()->d3ddev;

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZENABLE, (BOOL)self->desc.depthTest);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZWRITEENABLE, (BOOL)self->desc.depthWrite);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_CULLMODE, self->desc.cull ? D3DCULL_CW : D3DCULL_NONE);
	
	if(self->desc.blend) {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLEND, CrGpuState_blendFactorMapping[self->desc.blendFactorSrcRGB - CrGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLEND, CrGpuState_blendFactorMapping[self->desc.blendFactorDestRGB - CrGpuState_BlendFactor_One]);
		
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLENDALPHA, CrGpuState_blendFactorMapping[self->desc.blendFactorSrcA - CrGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLENDALPHA, CrGpuState_blendFactorMapping[self->desc.blendFactorDestA - CrGpuState_BlendFactor_One]);
		
	}
	else {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, FALSE);
	}

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_FILLMODE, CrGpuState_polygonModeMapping[self->desc.polygonMode - CrGpuState_PolygonMode_Line]);
}
