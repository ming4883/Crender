#include "Material.h"

#include "../lib/crender/StrUtil.h"
#include "../lib/crender/Shader.h"

#include <stdlib.h>
#include <string.h>

enum
{
	VS,
	TC,
	TE,
	GS,
	FS,
	ShaderCount
};

Material* materialAlloc()
{
	Material* self = malloc(sizeof(Material));
	memset(self, 0, sizeof(Material));

	return self;
}

CrBool materialInitWithShaders(Material* self, const char** args)
{
	const char* key; const char* val;
	int i = 0;

	while(1) {
		key = args[i++];
		if(nullptr == key) break;

		val = args[i++];
		if(nullptr == val) break;

		if(0 == strcasecmp(key, "vs")) {
			self->shaders[VS] = crGpuShaderAlloc();
			crGpuShaderInit(self->shaders[VS], &val, 1, CrGpuShaderType_Vertex);
		}

		if(0 == strcasecmp(key, "fs")) {
			self->shaders[FS] = crGpuShaderAlloc();
			crGpuShaderInit(self->shaders[FS], &val, 1, CrGpuShaderType_Fragment);
		}
		
		if(0 == strcasecmp(key, "tc")) {
			self->shaders[TC] = crGpuShaderAlloc();
			crGpuShaderInit(self->shaders[TC], &val, 1, CrGpuShaderType_TessControl);
		}

		if(0 == strcasecmp(key, "te")) {
			self->shaders[TE] = crGpuShaderAlloc();
			crGpuShaderInit(self->shaders[TE], &val, 1, CrGpuShaderType_TessEvaluation);
		}

		if(0 == strcasecmp(key, "gs")) {
			self->shaders[GS] = crGpuShaderAlloc();
			crGpuShaderInit(self->shaders[GS], &val, 1, CrGpuShaderType_Geometry);
		}
	}
	
	// at least we need 1 vertex shader and 1 fragment shader
	if(nullptr == self->shaders[VS] || nullptr == self->shaders[FS])
		return CrFalse;

	self->program = crGpuProgramAlloc();
	crGpuProgramInit(self->program, self->shaders, ShaderCount);

	if(!(self->program->flags & CrGpuProgram_Inited))
		return CrFalse;

	self->flags |= MaterialFlag_Inited;

	return CrTrue;
}


void materialFree(Material* self)
{
	int i;

	if(nullptr == self)
		return;

	if(nullptr != self->program)
		crGpuProgramFree(self->program);

	for(i=0; i<ShaderCount; ++i)
	{
		if(nullptr != self->shaders[i])
			crGpuShaderFree(self->shaders[i]);
	}

	free(self);
}
