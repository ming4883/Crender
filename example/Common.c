#include "Common.h"

#include "../lib/glsw/glsw.h"

glswFileSystem myFileSystem = {crRead, crOpen, crClose};
InputStream myInputStream = {crRead, crOpen, crClose};

AppContext* appAlloc()
{
	AppContext* self = malloc(sizeof(AppContext));
	memset(self, 0, sizeof(AppContext));
	return self;
}

void appInit(AppContext* self)
{
	self->gpuState = crGpuStateAlloc();
	crGpuStateInit(self->gpuState);

	self->renderTarget = crRenderTargetAlloc();
	crRenderTargetInit(self->renderTarget);

	self->istream = &myInputStream;

	self->aspect.width = (float)crAppContext.context->xres;
	self->aspect.height = (float)crAppContext.context->yres;

	crRenderTargetSetViewport(0, 0, self->aspect.width, self->aspect.height, -1, 1);

	crDbgStr("crender started with %d x %d, api=%s\n", crAppContext.context->xres, crAppContext.context->yres, crAppContext.context->apiName);
}

void appFree(AppContext* self)
{
	crRenderTargetFree(self->renderTarget);
	crGpuStateFree(self->gpuState);
	free(self);
}

void appLoadMaterialBegin(AppContext* self, const char** directives)
{
	glswInit(&myFileSystem);

	if(strcmp("gl", crAppContext.context->apiName) == 0) {
		glswSetPath("", ".glsl");
		if(3 == crAppContext.context->apiMajorVer) {
			glswAddDirectiveToken("", "#version 150");
		}
		else if(4 == crAppContext.context->apiMajorVer) {
			glswAddDirectiveToken("", "#version 400");
		}
	}
	else if(strcmp("gles", crAppContext.context->apiName) == 0) {
		glswSetPath("", ".gles");
	}
	else if(strcmp("d3d9", crAppContext.context->apiName) == 0) {
		glswSetPath("", ".hlsl");
	}

	if(nullptr != directives) {
		int i = 0;
		const char* key;
		const char* val;

		while(1) {
			key = directives[i++];
			if(nullptr == key) break;

			val = directives[i++];
			if(nullptr == val) break;

			glswAddDirectiveToken(key, val);
		}
	}
}

void appLoadMaterialEnd(AppContext* self)
{
	glswShutdown();
}

Material* appLoadMaterial(const char* vsKey, const char* fsKey, const char* tcKey, const char* teKey, const char* gsKey)
{
	const char* args[11] = {nullptr};
	size_t idx = 0;
	Material* material = nullptr;

	if(nullptr != vsKey) {
		args[idx++] = "vs";
		args[idx++] = glswGetShader(vsKey);
	}

	if(nullptr != tcKey) {
		args[idx++] = "tc";
		args[idx++] = glswGetShader(tcKey);
	}

	if(nullptr != teKey) {
		args[idx++] = "te";
		args[idx++] = glswGetShader(teKey);
	}

	if(nullptr != gsKey) {
		args[idx++] = "gs";
		args[idx++] = glswGetShader(gsKey);
	}

	if(nullptr != fsKey) {
		args[idx++] = "fs";
		args[idx++] = glswGetShader(fsKey);
	}

	material = materialAlloc();

	materialInitWithShaders(material, args);

	if(0 == (material->flags & MaterialFlag_Inited))
		crDbgStr("failed to load material vs=%s,fs=%s!\n", vsKey, fsKey);

	return material;
}

void appShaderContextPreRender(AppContext* self, Material* material)
{
	ShaderContext shdcontext = self->shaderContext;

	// gles does not support transpose matrix, so we have to DIY
	crMat44Transpose(&shdcontext.worldViewMtx, &shdcontext.worldViewMtx);
	crMat44Transpose(&shdcontext.worldViewProjMtx, &shdcontext.worldViewProjMtx);
	crMat44Transpose(&shdcontext.worldMtx, &shdcontext.worldMtx);

	crGpuProgramUniformMtx4fv(material->program, CrHash("u_worldViewMtx"), 1, CrFalse, shdcontext.worldViewMtx.v);
	crGpuProgramUniformMtx4fv(material->program, CrHash("u_worldViewProjMtx"), 1, CrFalse, shdcontext.worldViewProjMtx.v);
	crGpuProgramUniformMtx4fv(material->program, CrHash("u_worldMtx"), 1, CrFalse, shdcontext.worldMtx.v);
	crGpuProgramUniform4fv(material->program, CrHash("u_matDiffuse"), 1, shdcontext.matDiffuse.v);
	crGpuProgramUniform4fv(material->program, CrHash("u_matSpecular"), 1, shdcontext.matSpecular.v);
	crGpuProgramUniform1fv(material->program, CrHash("u_matShininess"), 1, &shdcontext.matShininess);
}


