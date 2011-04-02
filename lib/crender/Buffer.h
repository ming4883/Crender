#ifndef __CRENDER_BUFFER_H__
#define __CRENDER_BUFFER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CrBufferType
{
	CrBufferType_Vertex,	//!< vertex buffer
	CrBufferType_Index,	//!< 16-bit index buffer
	CrBufferType_Index8,	//!< 8-bit index buffer

#if !defined(CR_GLES_2)
	CrBufferType_Index32,	//!< 32-bit index buffer
	CrBufferType_Uniform,	//!< shader uniform buffer
#endif

} CrBufferType;

typedef enum CrBufferFlag
{
	CrBuffer_Inited = 0x0001,
	CrBuffer_Mapped = 0x0002,
} CrBufferFlag;

typedef enum CrBufferMapAccess
{
	CrBufferMapAccess_Read,
	CrBufferMapAccess_Write,
	CrBufferMapAccess_ReadWrite,
} CrBufferMapAccess;

typedef struct CrBuffer
{
	size_t flags;	// combinations of CrBufferFlag
	CrBufferType type;
	size_t sizeInBytes;

} CrBuffer;

CrBuffer* crBufferAlloc();

void crBufferFree(CrBuffer* self);

CrBool crBufferInit(CrBuffer* self, CrBufferType type, size_t sizeInBytes, void* initialData);

void crBufferUpdate(CrBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data);

void* crBufferMap(CrBuffer* self, CrBufferMapAccess usage);

void crBufferUnmap(CrBuffer* self);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_BUFFER_H__