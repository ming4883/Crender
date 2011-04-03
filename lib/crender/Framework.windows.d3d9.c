#include "Framework.h"

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "API.d3d9.h"

CrAppContext crAppContext = {
	"crApp",
	"d3d9",
	9, 0,
	CrFalse,
	CrFalse,
	853,
	480,
};

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE ignoreMe0, LPSTR ignoreMe1, INT ignoreMe2)
{
    LPCSTR szName = "CrApp";
    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(0), 0, 0, 0, 0, szName, 0 };
    DWORD dwStyle = WS_SYSMENU | WS_VISIBLE | WS_POPUP;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT rect;
    int windowWidth, windowHeight, windowLeft, windowTop;
    HWND hWnd;
    DWORD previousTime = GetTickCount();
    MSG msg = {0};
	DWORD accumTime = 0;
	DWORD accumFrame = 0;
	
    crAppConfig();

    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClassExA(&wc);

	SetRect(&rect, 0, 0, crAppContext.xres, crAppContext.yres);
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    windowWidth = rect.right - rect.left;
    windowHeight = rect.bottom - rect.top;
    windowLeft = GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2;
    windowTop = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2;
    hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);
	SetWindowTextA(hWnd, crAppContext.appName);

	// initialize d3d9
	crAPI.d3d = Direct3DCreate9(D3D_SDK_VERSION);

	{
		D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information
		HRESULT hr;

		memset(&d3dpp, 0, sizeof(d3dpp));    // clear out the struct for use
		d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
		d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferWidth = crAppContext.xres;
		d3dpp.BackBufferHeight = crAppContext.yres;
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.EnableAutoDepthStencil = TRUE;

		if(crAppContext.vsync) {
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else {
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		
		// create a device class using this information and information from the d3dpp stuct
		hr = IDirect3D9_CreateDevice(crAPI.d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&d3dpp,
			&crAPI.d3ddev);

		if(FAILED(hr)) {
			crDbgStr("using software vertex processing device...");
			hr = IDirect3D9_CreateDevice(crAPI.d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				hWnd,
				D3DCREATE_PUREDEVICE | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&d3dpp,
				&crAPI.d3ddev);

			if(FAILED(hr)) {
				crDbgStr("even software vertex processing device is not support :-(");
				return 0;
			}
		}

		IDirect3DDevice9_GetBackBuffer(crAPI.d3ddev, 0, 0, D3DBACKBUFFER_TYPE_MONO, &crAPI.d3dcolorbuf);
		IDirect3DDevice9_GetDepthStencilSurface(crAPI.d3ddev, &crAPI.d3ddepthbuf);
	}

	crAppInitialize();
    
    // -------------------
    // Start the Game Loop
    // -------------------
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DWORD currentTime = GetTickCount();
            DWORD deltaTime = currentTime - previousTime;
			previousTime = currentTime;

			accumTime += deltaTime;
			accumFrame += 1;

			while(accumTime >= 4000) {
				crDbgStr("fps = %.0f\n", accumFrame * 0.25f);
				accumTime -= 4000;
				accumFrame = 0;
			}

            crAppUpdate(deltaTime);

			IDirect3DDevice9_BeginScene(crAPI.d3ddev);

            crAppRender();

			IDirect3DDevice9_EndScene(crAPI.d3ddev);

			IDirect3DDevice9_Present(crAPI.d3ddev, nullptr, nullptr, nullptr, nullptr);
        }
    }

	crAppFinalize();

	IDirect3DSurface9_Release(crAPI.d3ddepthbuf);
	IDirect3DSurface9_Release(crAPI.d3dcolorbuf);
	IDirect3DDevice9_Release(crAPI.d3ddev);
	IDirect3D9_Release(crAPI.d3d);

    UnregisterClassA(szName, wc.hInstance);

    return 0;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    switch (msg)
    {
        case WM_LBUTTONUP:
            crAppHandleMouse(x, y, CrApp_MouseUp);
            break;

        case WM_LBUTTONDOWN:
            crAppHandleMouse(x, y, CrApp_MouseDown);
            break;

        case WM_MOUSEMOVE:
            crAppHandleMouse(x, y, CrApp_MouseMove);
            break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
                case VK_OEM_2: // Question Mark / Forward Slash for US Keyboards
                    break;
            }
            break;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
