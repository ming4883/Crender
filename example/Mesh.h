#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

#include "Stream.h"
#include "../lib/crender/Platform.h"
#include "../lib/crender/Vec3.h"
#include "../lib/crender/Vec2.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrBuffer;
struct CrGpuProgram;

enum MeshTrait
{
	MeshTrait_MaxTexcoord = 2
};

typedef struct MeshData
{
	unsigned char* buffer;
	size_t sizeInBytes;
	char shaderName[16];
} MeshData;

enum MeshFlag
{
	MeshFlag_Inited = 1 << 0,
	MeshFlag_Dirty = 1 << 1,
};

typedef struct Mesh
{
	size_t flags;
	size_t vertexCount;
	size_t indexCount;
	size_t vertexPerPatch;
	struct MeshData index;
	struct MeshData vertex;
	struct MeshData normal;
	struct MeshData color;
	struct MeshData texcoord[MeshTrait_MaxTexcoord];
} Mesh;

Mesh* meshAlloc();

void meshFree(Mesh* self);

void meshInit(Mesh* self, size_t vertexCount, size_t indexCount);

void meshInitWithUnitSphere(Mesh* self, size_t segmentCount);

void meshInitWithQuad(Mesh* self, float width, float height, const CrVec3* offset, const CrVec2* uvScale, size_t segmentCount);

void meshInitWithScreenQuad(Mesh* self);

CrBool meshInitWithObjFile(Mesh* self, const char* path, InputStream* stream);

void meshCommit(Mesh* self);

void meshPreRender(Mesh* self, struct CrGpuProgram* program);

void meshRenderTriangles(Mesh* self);

void meshRenderPatches(Mesh* self);

void meshRenderPoints(Mesh* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
