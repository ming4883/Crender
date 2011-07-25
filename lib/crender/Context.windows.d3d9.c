#include "Context.d3d9.h"
#include "Mem.h"

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
