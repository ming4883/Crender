#include "Context.d3d9.h"
#include "Mem.h"
#include "Mat44.h"
#include "Texture.d3d9.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

CrContextImpl* current = nullptr;

CR_API CrContext* crContext()
{
	return &current->i;
}

CR_API CrContextImpl* crContextImpl()
{
	return current;
}

CR_API CrBool crContextFixedPipelineOnly()
{
	return 0 == current->i.apiMinorVer;
}

CR_API CrContext* crContextAlloc()
{
	CrContextImpl* self = crMem()->alloc(sizeof(CrContextImpl), "CrContext");
	memset(self, 0, sizeof(CrContextImpl));

	self->i.apiName = "d3d9";
	self->i.apiMajorVer = 9;
	self->i.apiMinorVer = 3;
	self->i.xres = 853;
	self->i.yres = 480;

	crGpuStateInit(&self->i.gpuState);
	crFfpStateInit(&self->i.ffpState);

	return &self->i;
}

CrBool crContextCreateFrameBuffers(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	HRESULT hr;

	if(nullptr != impl->d3dcolorbuf)
		IDirect3DSurface9_Release(impl->d3dcolorbuf);

	if(nullptr != impl->d3ddepthbuf)
		IDirect3DSurface9_Release(impl->d3ddepthbuf);

	if(nullptr != impl->d3dswapchain)
		IDirect3DSwapChain9_Release(impl->d3dswapchain);

	// create depth buffer
	hr = IDirect3DDevice9_CreateDepthStencilSurface(
		impl->d3ddev,
		self->xres, self->yres, // size
		D3DFMT_D24S8, // format
		D3DMULTISAMPLE_NONE, 0, // multisampling
		TRUE, // discard
		&impl->d3ddepthbuf,
		nullptr);

	if(FAILED(hr)) {
		crDbgStr("failed to create depth buffer");
		return CrFalse;
	}

	// create additional swap chain (with color buffer) for easy window resize handling
	{
		D3DPRESENT_PARAMETERS d3dpp;
		memset(&d3dpp, 0, sizeof(d3dpp)); // clear out the struct for use
		d3dpp.Windowed = TRUE; // program windowed, not fullscreen
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; // discard old frames
		d3dpp.hDeviceWindow = impl->hwnd; // set the window to be used by Direct3D
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferWidth = self->xres;
		d3dpp.BackBufferHeight = self->yres;
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp.EnableAutoDepthStencil = FALSE;
		d3dpp.PresentationInterval = self->vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	
		hr = IDirect3DDevice9_CreateAdditionalSwapChain(impl->d3ddev, &d3dpp, &impl->d3dswapchain);

		if(FAILED(hr)) {
			crDbgStr("failed to create additional swap chain");
			return CrFalse;
		}

		hr = IDirect3DSwapChain9_GetBackBuffer(impl->d3dswapchain, 0, D3DBACKBUFFER_TYPE_MONO, &impl->d3dcolorbuf);

		if(FAILED(hr)) {
			crDbgStr("failed to get color buffer from additional swap chain");
			return CrFalse;
		}
	}

	// apply frame buffers
	hr = IDirect3DDevice9_SetRenderTarget(impl->d3ddev, 0, impl->d3dcolorbuf);
	if(FAILED(hr)) {
		crDbgStr("failed to set color buffer");
		return CrFalse;
	}

	hr = IDirect3DDevice9_SetDepthStencilSurface(impl->d3ddev, impl->d3ddepthbuf);
	if(FAILED(hr)) {
		crDbgStr("failed to set depth buffer");
		return CrFalse;
	}

	return CrTrue;
}

CR_API CrBool crContextInit(CrContext* self, void** window)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	HWND* hWnd = (HWND*)window;
	WINDOWINFO info;

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information
	HRESULT hr;

	GetWindowInfo(*hWnd, &info);
	self->xres = info.rcClient.right - info.rcClient.left;
	self->yres = info.rcClient.bottom - info.rcClient.top;

	impl->hwnd = *hWnd;
	impl->d3d = Direct3DCreate9(D3D_SDK_VERSION);

	memset(&d3dpp, 0, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = *hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = self->xres;
	d3dpp.BackBufferHeight = self->yres;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.EnableAutoDepthStencil = FALSE;
	d3dpp.PresentationInterval = self->vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	
	// create a device class using this information and information from the d3dpp stuct
	hr = IDirect3D9_CreateDevice(impl->d3d,
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		*hWnd,
		D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&impl->d3ddev);

	if(FAILED(hr)) {
		crDbgStr("using software vertex processing device...");
		hr = IDirect3D9_CreateDevice(impl->d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			*hWnd,
			D3DCREATE_PUREDEVICE | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&impl->d3ddev);

		if(FAILED(hr)) {
			crDbgStr("even software vertex processing device is not support :-(");
			return CrFalse;
		}

		self->apiMinorVer = 0;
	}

	if(CrFalse == crContextCreateFrameBuffers(self)) {
		return CrFalse;
	}

	current = impl;

	return CrTrue;
}

CR_API void crContextFree(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;

	current = nullptr;

	IDirect3DSwapChain9_Release(impl->d3dswapchain);
	IDirect3DSurface9_Release(impl->d3ddepthbuf);
	IDirect3DSurface9_Release(impl->d3dcolorbuf);
	IDirect3DDevice9_Release(impl->d3ddev);
	IDirect3D9_Release(impl->d3d);

	crMem()->free(self, "CrContext");
}

CR_API CrBool crContextPreRender(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	return S_OK == IDirect3DDevice9_BeginScene(impl->d3ddev);
}

CR_API void crContextPostRender(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	IDirect3DDevice9_EndScene(impl->d3ddev);
}

CR_API void crContextSwapBuffers(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	//IDirect3DDevice9_Present(impl->d3ddev, nullptr, nullptr, nullptr, nullptr);
	IDirect3DSwapChain9_Present(impl->d3dswapchain, nullptr, nullptr, impl->hwnd, nullptr, 0);
}

CR_API CrBool crContextChangeResolution(CrContext* self, size_t xres, size_t yres)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	HWND hWnd = (HWND)impl->hwnd;
	RECT rcClient, rcWindow;
	POINT ptDiff;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	SetWindowPos(hWnd, 0, rcWindow.left, rcWindow.top, xres + ptDiff.x, yres + ptDiff.y, SWP_NOMOVE|SWP_NOZORDER);

	self->xres = xres;
	self->yres = yres;

	return crContextCreateFrameBuffers(self);;
}

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


CR_API void crContextApplyGpuState(CrContext* self)
{
	IDirect3DDevice9* d3ddev = current->d3ddev;

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZENABLE, (BOOL)self->gpuState.depthTest);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ZWRITEENABLE, (BOOL)self->gpuState.depthWrite);
	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_CULLMODE, self->gpuState.cull ? D3DCULL_CW : D3DCULL_NONE);
	
	if(self->gpuState.blend) {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SEPARATEALPHABLENDENABLE, TRUE);

		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLEND, CrGpuState_blendFactorMapping[self->gpuState.blendFactorSrcRGB - CrGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLEND, CrGpuState_blendFactorMapping[self->gpuState.blendFactorDestRGB - CrGpuState_BlendFactor_One]);
		
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_SRCBLENDALPHA, CrGpuState_blendFactorMapping[self->gpuState.blendFactorSrcA - CrGpuState_BlendFactor_One]);
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_DESTBLENDALPHA, CrGpuState_blendFactorMapping[self->gpuState.blendFactorDestA - CrGpuState_BlendFactor_One]);
		
	}
	else {
		IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_ALPHABLENDENABLE, FALSE);
	}

	IDirect3DDevice9_SetRenderState(d3ddev, D3DRS_FILLMODE, CrGpuState_polygonModeMapping[self->gpuState.polygonMode - CrGpuState_PolygonMode_Line]);
}
/**/

CR_API void crContextApplyFfpState(CrContext* self)
{
}


CR_API void crContextSetViewport(CrContext* self, float x, float y, float w, float h, float zmin, float zmax)
{
	IDirect3DDevice9* d3ddev = current->d3ddev;

	D3DVIEWPORT9 vp;
	vp.X = (DWORD)x;
	vp.Y = (DWORD)y;
	vp.Width  = (DWORD)w;
	vp.Height = (DWORD)h;
	vp.MinZ = zmin * 0.5f + 0.5f;
	vp.MaxZ = zmax * 0.5f + 0.5f;;

	IDirect3DDevice9_SetViewport(d3ddev, &vp);
}

CR_API void crContextClearColor(CrContext* self, float r, float g, float b, float a)
{
	IDirect3DDevice9* d3ddev = current->d3ddev;
	IDirect3DDevice9_Clear(d3ddev, 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(r, g, b, a), 1, 0);
}

CR_API void crContextClearDepth(CrContext* self, float z)
{
	IDirect3DDevice9* d3ddev = current->d3ddev;
	IDirect3DDevice9_Clear(d3ddev, 0, nullptr, D3DCLEAR_ZBUFFER, 0, z, 0);
}

CR_API CrBool crContextPreRTT(CrContext* self, struct CrTexture** colors, struct CrTexture* depth)
{
	CrContextImpl* impl = (CrContextImpl*)self;

	size_t bufCnt;
	CrTexture** curr;
	if(nullptr == self)
		return CrFalse;
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = colors;
		while(*curr != nullptr) {
			CrTextureImpl* buf = (CrTextureImpl*)*curr;
			IDirect3DDevice9_SetRenderTarget(impl->d3ddev, bufCnt, buf->d3dsurf);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		CrTextureImpl* buf = (CrTextureImpl*)depth;
		IDirect3DDevice9_SetDepthStencilSurface(impl->d3ddev, buf->d3dsurf);
	}
	else {
		IDirect3DDevice9_SetDepthStencilSurface(impl->d3ddev, nullptr);
	}

	return CrTrue;
}

CR_API CrBool crContextPostRTT(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;

	IDirect3DDevice9_SetRenderTarget(impl->d3ddev, 0, impl->d3dcolorbuf);
	IDirect3DDevice9_SetDepthStencilSurface(impl->d3ddev, impl->d3ddepthbuf);

	return CrTrue;
}
