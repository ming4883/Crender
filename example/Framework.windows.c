#include "Framework.h"

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

CrAppContext crAppContext = {
	"crApp", nullptr
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

#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
#endif

	// allocate CrContext and config
	crAppContext.context = crContextAlloc();
	crAppConfig();

	wc.hCursor = LoadCursor(0, IDC_ARROW);
	RegisterClassExA(&wc);

	SetRect(&rect, 0, 0, crAppContext.context->xres, crAppContext.context->yres);
	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
	windowWidth = rect.right - rect.left;
	windowHeight = rect.bottom - rect.top;
	windowLeft = GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2;
	windowTop = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2;
	hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);
	SetWindowTextA(hWnd, crAppContext.appName);

	// initialize CrContext
	if(crContextInit(crAppContext.context, &hWnd)) {

		crAppInitialize();

		// -------------------
		// Start the Game Loop
		// -------------------
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
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

				crContextPreRender(crAppContext.context);

				crAppRender();

				crContextPostRender(crAppContext.context);

				crContextSwapBuffers(crAppContext.context);

				Sleep(1000 / 60);
			}
		}

		crAppFinalize();
	}

	crContextFree(crAppContext.context);

	UnregisterClassA(szName, wc.hInstance);

	return 0;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	switch (msg) {
	case WM_LBUTTONUP:
		crAppHandleMouse(x, y, CrApp_MouseUp);
		break;

	case WM_LBUTTONDOWN:
		crAppHandleMouse(x, y, CrApp_MouseDown);
		break;

	case WM_MOUSEMOVE:
		crAppHandleMouse(x, y, CrApp_MouseMove);
		break;

	case WM_KEYDOWN: {
			switch (wParam) {
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

void* crOpen(const char* filename)
{
	static char buf[512];

	static const char* paths[] = {
		"../../example/",
		"../example/",
		"../../media/",
		"../media/",
		"media",
		nullptr,
	};

	FILE* fp;
	const char** path;

	if(nullptr != (fp = fopen(filename, "rb")))
		return fp;

	for(path = paths; nullptr != *path; ++path) {
		strcpy(buf, *path);
		if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
			return fp;
	}

	crDbgStr("failed to open asset '%s'!\n", filename);

	return nullptr;
}

void crClose(void* handle)
{
	fclose((FILE*)handle);
}

size_t crRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return fread(buff, elsize, nelem, (FILE*)handle);
}