#ifndef __EXAMPLE_COMMON_H__
#define __EXAMPLE_COMMON_H__

#include "../lib/crender/Platform.h"
#include "../lib/crender/Vec3.h"
#include "../lib/crender/Vec4.h"
#include "../lib/crender/Mat44.h"
#include "../lib/crender/Shader.h"
#include "../lib/crender/RenderTarget.h"

#include "Framework.h"
#include "Material.h"
#include "Stream.h"

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ShaderContext
{
	CrMat44 worldViewProjMtx;
	CrMat44 worldViewMtx;
	CrMat44 worldMtx;
	CrVec4 matDiffuse;
	CrVec4 matSpecular;
	float matShininess;
} ShaderContext;

typedef struct AppContext
{
	struct
	{
		float width;
		float height;
	} aspect;

	ShaderContext shaderContext;

	CrRenderTarget* renderTarget;

	InputStream* istream;

} AppContext;

AppContext* appAlloc();
void appInit(AppContext* self);
void appFree(AppContext* self);
void appResize(AppContext* self, size_t xres, size_t yres);

void appLoadMaterialBegin(AppContext* self, const char** directives);
Material* appLoadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey);
void appLoadMaterialEnd(AppContext* self);

void appShaderContextPreRender(AppContext* self, Material* material);

#ifdef __cplusplus
}
#endif


#endif	// __EXAMPLE_COMMON_H__
