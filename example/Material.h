#ifndef __EXAMPLE_MATERIAL_H__
#define __EXAMPLE_MATERIAL_H__

#include "../lib/crender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrGpuShader;
struct CrGpuProgram;

typedef enum MaterialFlag
{
	MaterialFlag_Inited = 1 << 0,
} MaterialFlag;

typedef struct Material
{
	struct CrGpuShader* shaders[8];
	struct CrGpuProgram* program;
	size_t flags;
} Material;

Material* materialAlloc();

void materialFree(Material* self);

CrBool materialInitWithShaders(Material* self, const char** args);

#ifdef __cplusplus
}
#endif

#endif // __EXAMPLE_MATERIAL_H__
