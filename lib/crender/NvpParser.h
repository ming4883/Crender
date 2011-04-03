#ifndef __CRENDER_NVPPARSER_H__
#define __CRENDER_NVPPARSER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrNvpParser
{
	char* mStr;
	char* mPos;
} CrNvpParser;

CR_API CrNvpParser* crNvpParserAlloc();

CR_API void crNvpParserFree(CrNvpParser* self);

CR_API void crNvpParserInit(CrNvpParser* self, const char* str);

CR_API CrBool crNvpParserNext(CrNvpParser* self, const char** name, const char** value);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_NVPPARSER_H__
