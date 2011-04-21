#include "Mesh.h"

#include "../lib/crender/Mem.h"
#include "../lib/crender/Buffer.h"
#include "../lib/crender/Shader.h"
#include "../lib/crender/Vec2.h"
#include "../lib/crender/Vec3.h"
#include "../lib/crender/Vec4.h"

#include <math.h>
#include <stdio.h>

typedef struct MeshImpl {
	Mesh i;
	struct CrBuffer* indexBuffer;
	struct CrBuffer* vertexBuffer;
	struct CrBuffer* normalBuffer;
	struct CrBuffer* colorBuffer;
	struct CrBuffer* tcBuffer[MeshTrait_MaxTexcoord];
	size_t gpuInputId;
} MeshImpl;

Mesh* meshAlloc()
{
	MeshImpl* self = crMem()->alloc(sizeof(MeshImpl), "mesh");
	memset(self, 0, sizeof(MeshImpl));

	return &self->i;
}

void meshInit(Mesh* self, size_t vertexCount, size_t indexCount)
{
	size_t i;
	MeshImpl* impl = (MeshImpl*)self;

	self->vertexCount = vertexCount;
	self->indexCount = indexCount;

	self->index.sizeInBytes = sizeof(unsigned short) * self->indexCount;
	self->index.buffer = malloc(self->index.sizeInBytes);
	strcpy(self->index.shaderName, "");

	self->vertex.sizeInBytes = sizeof(CrVec3) * self->vertexCount;
	self->vertex.buffer = malloc(self->vertex.sizeInBytes);
	strcpy(self->vertex.shaderName, "i_vertex");

	self->normal.sizeInBytes = sizeof(CrVec3) * self->vertexCount;
	self->normal.buffer = malloc(self->normal.sizeInBytes);
	strcpy(self->normal.shaderName, "i_normal");

	self->color.sizeInBytes = sizeof(CrVec4) * self->vertexCount;
	self->color.buffer = malloc(self->color.sizeInBytes);
	strcpy(self->color.shaderName, "i_color");

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->texcoord[i].sizeInBytes = sizeof(CrVec2) * self->vertexCount;
		self->texcoord[i].buffer = malloc(self->texcoord[i].sizeInBytes);
		sprintf(self->texcoord[i].shaderName, "i_texcoord%d", (int)i);
	}

	// hardware buffer
	impl->indexBuffer = crBufferAlloc();
	crBufferInit(impl->indexBuffer, CrBufferType_Index, self->index.sizeInBytes, nullptr);

	impl->vertexBuffer = crBufferAlloc();
	crBufferInit(impl->vertexBuffer, CrBufferType_Vertex, self->vertex.sizeInBytes, nullptr);

	impl->normalBuffer = crBufferAlloc();
	crBufferInit(impl->normalBuffer, CrBufferType_Vertex, self->normal.sizeInBytes, nullptr);

	impl->colorBuffer = crBufferAlloc();
	crBufferInit(impl->colorBuffer, CrBufferType_Vertex, self->color.sizeInBytes, nullptr);

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		impl->tcBuffer[i] = crBufferAlloc();
		crBufferInit(impl->tcBuffer[i], CrBufferType_Vertex, self->texcoord[i].sizeInBytes, nullptr);
	}

	impl->gpuInputId = crGenGpuInputId();

	self->flags |= MeshFlag_Inited;
}

void meshFree(Mesh* self)
{
	size_t i;
	MeshImpl* impl = (MeshImpl*)self;

	if(self->flags & MeshFlag_Inited) {

		crBufferFree(impl->indexBuffer);
		crBufferFree(impl->vertexBuffer);
		crBufferFree(impl->normalBuffer);
		crBufferFree(impl->colorBuffer);

		for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
			crBufferFree(impl->tcBuffer[i]);
		}

		free(self->index.buffer);
		free(self->vertex.buffer);
		free(self->normal.buffer);
		free(self->color.buffer);

		for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
			free(self->texcoord[i].buffer);
		}
	}

	free(self);
}

void meshCommit(Mesh* self)
{
	size_t i;
	MeshImpl* impl = (MeshImpl*)self;

#define commit(x) {\
	crBufferUpdate(impl->x##Buffer, 0, self->x.sizeInBytes, self->x.buffer);\
	}
	if(nullptr == self)
		return;

	if(0 == (self->flags & MeshFlag_Inited))
		return;

	commit(index);
	commit(vertex);
	commit(normal);
	commit(color);
	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		crBufferUpdate(impl->tcBuffer[i], 0, self->texcoord[i].sizeInBytes, self->texcoord[i].buffer);
	}

#undef commit
}

void meshPreRender(Mesh* self, struct CrGpuProgram* program)
{
	MeshImpl* impl = (MeshImpl*)self;

	CrGpuProgramInput inputs[] = {
		{impl->indexBuffer},
		{impl->vertexBuffer, self->vertex.shaderName, 0, sizeof(CrVec3), CrGpuFormat_FloatR32G32B32},
		{impl->normalBuffer, self->normal.shaderName, 0, sizeof(CrVec3), CrGpuFormat_FloatR32G32B32},
		{impl->colorBuffer, self->color.shaderName, 0, sizeof(CrVec4), CrGpuFormat_FloatR32G32B32A32},
		{impl->tcBuffer[0], self->texcoord[0].shaderName, 0, sizeof(CrVec2), CrGpuFormat_FloatR32G32},
		{impl->tcBuffer[1], self->texcoord[1].shaderName, 0, sizeof(CrVec2), CrGpuFormat_FloatR32G32},
	};

	crGpuProgramBindInput(program, impl->gpuInputId, inputs, crCountOf(inputs));
}

void meshRenderTriangles(Mesh* self)
{
	crGpuDrawTriangleIndexed(0, self->indexCount, 0, self->vertexCount-1, 0);
}

void meshRenderPatches(Mesh* self)
{
	if((self->indexCount % self->vertexPerPatch) != 0)
		return;

	crGpuDrawPatchIndexed(0, self->indexCount, 0, self->vertexCount-1, 0, self->vertexPerPatch);
}

void meshRenderPoints(Mesh* self)
{
	crGpuDrawPoint(0, self->vertexCount);
}

void meshInitWithUnitSphere(Mesh* self, size_t segmentCount)
{
#define PI 3.14159265358979323846f

	MeshImpl* impl = (MeshImpl*)self;

	CrVec3* pos;
	CrVec3* nor;
	CrVec2* uv0;

	unsigned short* idx;

	float theta, phi;
	int i, j, t;

	int width = segmentCount * 2;
	int height = segmentCount;

	meshInit(self, (height-2)* width+2, (height-2)*(width-1)*2 * 3);
	self->vertexPerPatch = 3;

	idx = crBufferMap(impl->indexBuffer, CrBufferMapAccess_Write);
	pos = crBufferMap(impl->vertexBuffer, CrBufferMapAccess_Write);
	nor = crBufferMap(impl->normalBuffer, CrBufferMapAccess_Write);
	uv0 = crBufferMap(impl->tcBuffer[0], CrBufferMapAccess_Write);

	for(t=0, j=1; j<height-1; ++j)
	{
		for(i=0; i<width; ++i)
		{
			uv0[t].x = (float)i/(width-1);
			uv0[t].y = (float)j/(height-1);

			theta = uv0[t].y * PI;
			phi   = uv0[t].x * PI*2;
			pos[t].x =  sinf(theta) * cosf(phi);
			pos[t].y =  cosf(theta);
			pos[t].z = -sinf(theta) * sinf(phi);
			nor[t] = pos[t];
			++t;
		}
	}
	pos[t] = crVec3(0, 1, 0); nor[t] = pos[t]; ++t;
	pos[t] = crVec3(0,-1, 0); nor[t] = pos[t]; ++t;

	for(t=0, j=0; j<height-3; ++j)
	{
		for(i=0; i<width-1; ++i)
		{
			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j+1)*width + i+1;
			idx[t++] = (j  )*width + i+1;

			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j+1)*width + i  ;
			idx[t++] = (j+1)*width + i+1;
		}
	}

	for( i=0; i<width-1; i++ )
	{
		idx[t++] = (height-2)*width;
		idx[t++] = i;
		idx[t++] = i+1;

		idx[t++] = (height-2)*width+1;
		idx[t++] = (height-3)*width + i+1;
		idx[t++] = (height-3)*width + i;

	}

	crBufferUnmap(impl->indexBuffer);
	crBufferUnmap(impl->vertexBuffer);
	crBufferUnmap(impl->normalBuffer);
	crBufferUnmap(impl->tcBuffer[0]);

#undef PI
}

void meshInitWithQuad(Mesh* self, float width, float height, const CrVec3* offset, size_t segmentCount)
{
	MeshImpl* impl = (MeshImpl*)self;

	CrVec3* pos;
	CrVec3* nor;
	CrVec2* uv0;
	unsigned short* idx;

	size_t r, c;
	size_t stride = segmentCount+1;

	meshInit(self, stride * stride, (stride-1) * (stride-1) * 6);
	self->vertexPerPatch = 3;

	idx = crBufferMap(impl->indexBuffer, CrBufferMapAccess_Write);
	pos = crBufferMap(impl->vertexBuffer, CrBufferMapAccess_Write);
	nor = crBufferMap(impl->normalBuffer, CrBufferMapAccess_Write);
	uv0 = crBufferMap(impl->tcBuffer[0], CrBufferMapAccess_Write);

	for(r=0; r<(stride-1); ++r)
	{
		for(c=0; c<(stride-1); ++c)
		{

			unsigned short p0 = (unsigned short)(r * stride + (c+1));
			unsigned short p1 = (unsigned short)((r+1) * stride + (c+1));
			unsigned short p2 = (unsigned short)(r * stride + c);
			unsigned short p3 = (unsigned short)((r+1) * stride + c);

			(*idx++) = p0;
			(*idx++) = p1;
			(*idx++) = p2;

			(*idx++) = p3;
			(*idx++) = p2;
			(*idx++) = p1;
		}
	}

	for(r=0; r<stride; ++r)
	{
		float y = offset->v[1] + height * (float)r / segmentCount;

		for(c=0; c<stride; ++c)
		{
			float x = offset->v[0] + width * (float)c / segmentCount;

			size_t i = r * stride + c;
			pos[i] = crVec3(x, y, offset->v[2]);
			nor[i] = crVec3(0, 0, 1);
			uv0[i].x = height * (float)r / segmentCount;
			uv0[i].y = width * (float)c / segmentCount;
		}
	}

	crBufferUnmap(impl->indexBuffer);
	crBufferUnmap(impl->vertexBuffer);
	crBufferUnmap(impl->normalBuffer);
	crBufferUnmap(impl->tcBuffer[0]);
}

void meshInitWithScreenQuad(Mesh* self)
{
	MeshImpl* impl = (MeshImpl*)self;

	CrVec3* pos;
	CrVec2* uv0;
	unsigned short* idx;

	meshInit(self, 4, 6);
	self->vertexPerPatch = 3;

	idx = crBufferMap(impl->indexBuffer, CrBufferMapAccess_Write);
	pos = crBufferMap(impl->vertexBuffer, CrBufferMapAccess_Write);
	uv0 = crBufferMap(impl->tcBuffer[0], CrBufferMapAccess_Write);

	(*idx++) = 0; (*idx++) = 1; (*idx++) = 2;
	(*idx++) = 3; (*idx++) = 2; (*idx++) = 1;

	pos[0] = crVec3(-1, 1, 0); uv0[0] = crVec2(0, 1);
	pos[1] = crVec3(-1,-1, 0); uv0[1] = crVec2(0, 0);
	pos[2] = crVec3( 1, 1, 0); uv0[2] = crVec2(1, 1);
	pos[3] = crVec3( 1,-1, 0); uv0[3] = crVec2(1, 0);

	crBufferUnmap(impl->indexBuffer);
	crBufferUnmap(impl->vertexBuffer);
	crBufferUnmap(impl->tcBuffer[0]);
}
