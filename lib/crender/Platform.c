#include "Platform.h"

#ifndef _MSC_VER
#define _TRUNCATE ((size_t)-1)
#endif

#include <stdlib.h>
#include <stdio.h>

#if defined(CR_WIN32)

#include <windows.h>

CR_API void crDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

	va_list a;
	va_start(a, str);

	_vsnprintf_s(msg, _countof(msg), _TRUNCATE, str, a);
	OutputDebugStringA(msg);
}

#elif defined(CR_ANDROID)

#include <android/log.h>

CR_API void crDbgStr(const char* str, ...)
{
	va_list args;

	va_start(args, str);
	__android_log_vprint(ANDROID_LOG_INFO, "crender", str, args);
	va_end(args);
}

#else

#include <stdarg.h>

CR_API void crDbgStr(const char* str, ...)
{
	char msg[1024] = {0};

	va_list a;
	va_start(a, str);

	vsprintf(msg, str, a);
	printf("%s", msg);

	va_end(a);
}

#endif	// CR_WIN32

extern CrApiPrivates crAPI;
CR_API CrApiPrivates* crApiPrivates()
{
	return &crAPI;
}
