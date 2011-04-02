#ifndef __EXAMPLE_LABEL_WINDOWS_H__
#define __EXAMPLE_LABEL_WINDOWS_H__

#include "Label.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LabelImpl
{
	HDC hdc;
	HBITMAP hbmp;
	size_t width;
	size_t height;
	wchar_t* text;
} LabelImpl;

#endif	// __EXAMPLE_LABEL_WINDOWS_H__
