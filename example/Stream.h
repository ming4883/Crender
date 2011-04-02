#ifndef __EXAMPLE_STREAM_H__
#define __EXAMPLE_STREAM_H__

#include "../lib/crender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (* StreamRead) (void* buff, size_t elsize, size_t nelem, void* handle);
typedef void* (* StreamOpen) (const char* filename);
typedef void (* StreamClose) (void* handle);

typedef struct InputStream
{
	StreamRead read;
	StreamOpen open;
	StreamClose close;
} InputStream;

char* Stream_gets(InputStream* self, char* str, int num, void* handle);

#ifdef __cplusplus
}
#endif


#endif	// __EXAMPLE_STREAM_H__
