#ifndef __CRENDER_FRAMEWORK_H__
#define __CRENDER_FRAMEWORK_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Cr is a platform abstraction layer for rendering;
// To create a cross-platform app, simply link against the Cr source and implement the following functions.
//
void crAppConfig();								// application configuration. i.e. fill in crAppContext
CrBool crAppInitialize();							// application initialization
void crAppRender();								// draw scene (Cr swaps the backbuffer for you)
void crAppUpdate(unsigned int milliseconds);		// receive elapsed time (e.g., update physics)
void crAppHandleMouse(int x, int y, int action);	// handle mouse action: PEZ_DOWN, PEZ_UP, or PEZ_MOVE
void crAppFinalize();								// application finalization

typedef struct CrAppContext
{
	const char* appName;
	const char* apiName;
	int apiMajorVer;
	int apiMinorVer;
	CrBool multiSampling;
	CrBool vsync;
	int xres;
	int yres;

} CrAppContext;

enum
{
	CrApp_MouseDown,
	CrApp_MouseUp,
	CrApp_MouseMove,
};

extern CrAppContext crAppContext;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_FRAMEWORK_H__