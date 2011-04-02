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

CrNvpParser* crNvpParserAlloc();

void crNvpParserFree(CrNvpParser* self);

void crNvpParserInit(CrNvpParser* self, const char* str);

CrBool crNvpParserNext(CrNvpParser* self, const char** name, const char** value);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_NVPPARSER_H__
