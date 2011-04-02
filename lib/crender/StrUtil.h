#ifndef __CRENDER_STRUTIL_H__
#define __CRENDER_STRUTIL_H__

#include "Platform.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CR_VC)
#define strcasecmp stricmp
#endif

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_STRUTIL_H__
