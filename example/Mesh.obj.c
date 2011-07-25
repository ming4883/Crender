#include "Mesh.h"

#include "../lib/crender/Vec2.h"
#include "../lib/crender/Vec3.h"
#include "../lib/crender/Vec4.h"
#include "../lib/crender/StrHash.h"
#include "../lib/crender/uthash/uthash.h"

#include <math.h>
#include <stdio.h>

typedef struct ObjBuffer
{
	char* data;
	size_t elemSz;
	size_t cap;
	size_t cnt;
} ObjBuffer;

typedef struct ObjVertex
{
	size_t v, t, n;
} ObjVertex;

typedef struct ObjFace
{
	ObjVertex vert[4];
} ObjFace;

void objBufferResize(ObjBuffer* buf)
{
	buf->data = realloc(buf->data, buf->cap * buf->elemSz);
}

char* objBufferAppend(ObjBuffer* buf)
{
	char* ptr;

	if(buf->cap == buf->cnt) {
		buf->cap *= 2;
		objBufferResize(buf);
	}

	ptr = buf->data + (buf->cnt * buf->elemSz);

	++buf->cnt;

	return ptr;
}

const char* objWhitespace = " \t\n\r";

void objReadVec3(CrVec3* ret)
{
	char* token;
	ret->x = 0;
	ret->y = 0;
	ret->z = 0;

	token = strtok(nullptr, objWhitespace);
	if(nullptr != token)
		ret->x = (float)atof(token);

	token = strtok(nullptr, objWhitespace);
	if(nullptr != token)
		ret->y = (float)atof(token);

	token = strtok(nullptr, objWhitespace);
	if(nullptr != token)
		ret->z = (float)atof(token);
}

size_t objReadFace(ObjFace* face)
{
	char* token;
	char vi[16] = {0};
	char vti[16] = {0};
	char vni[16] = {0};
	size_t vcnt = 0;
	memset(face, 0, sizeof(ObjFace));

	while(nullptr != (token = strtok(nullptr, objWhitespace)) && vcnt <= 4) {
		if(strstr(token, "//")) {
			sscanf(token, "%[^/]//%[^/]", vi, vni);
			if(strlen(vi) > 0) face->vert[vcnt].v = atoi(vi)-1;
			if(strlen(vni) > 0) face->vert[vcnt].n = atoi(vni)-1;
		}
		else {
			sscanf(token, "%[^/]/%[^/]/%[^/]", vi, vti, vni);
			if(strlen(vi) > 0) face->vert[vcnt].v = atoi(vi)-1;
			if(strlen(vti) > 0) face->vert[vcnt].t = atoi(vti)-1;
			if(strlen(vni) > 0) face->vert[vcnt].n = atoi(vni)-1;
		}
		++vcnt;
	}

	return vcnt;
}


typedef struct ObjVertexEntry
{
	CrHashCode hash;
	size_t idx;
	UT_hash_handle hh;
} ObjVertexEntry;

typedef struct ObjVertexCache
{
	ObjVertexEntry* entryBuf;
	ObjVertex* vertexBuf;
	size_t* indexBuf;

	ObjVertexEntry* cache;

	size_t vertexCnt;
	size_t indexCnt;

} ObjVertexCache;

void objVertexCacheInit(ObjVertexCache* self, ObjFace* faces, size_t faceCnt, size_t vertexPreFace)
{
	size_t f, v;

	memset(self, 0, sizeof(ObjVertexCache));
	self->entryBuf = malloc(sizeof(ObjVertexEntry) * faceCnt * 3);
	self->vertexBuf = malloc(sizeof(ObjVertex) * faceCnt * 3);
	self->indexBuf = malloc(sizeof(size_t) * faceCnt * 3);

	for(f = 0; f < faceCnt; ++f) {
		ObjFace* face = &faces[f];
		for(v = 0; v < vertexPreFace; ++v) {

			ObjVertexEntry* entry;
			ObjVertex* vert = &face->vert[v];
			CrHashCode hash = CrHashStruct(vert, sizeof(ObjVertex));
			//CrHashCode hash = (vert->n << 16) + (vert->t << 8) + vert->v;

			HASH_FIND_INT(self->cache, &hash, entry);

			if(nullptr == entry) {
				// vertex not exists in cache, create a new entry in cache, add it to vertexBuf
				self->vertexBuf[self->vertexCnt] = face->vert[v];

				entry = &self->entryBuf[self->vertexCnt];
				entry->hash = hash;
				entry->idx = self->vertexCnt;

				HASH_ADD_INT(self->cache, hash, entry);

				++self->vertexCnt;
			}

			// check for hash collision
			if(0 != memcmp(&self->vertexBuf[entry->idx], vert, sizeof(ObjVertex)))
			{
				ObjVertex* a = &self->vertexBuf[entry->idx];
				ObjVertex* b = vert;
				crDbgStr("hash collision between {0x%08x, 0x%08x, 0x%08x}; {0x%08x, 0x%08x, 0x%08x}\n",
					a->v, a->t, a->n,
					b->v, b->t, b->n);
			}

			self->indexBuf[self->indexCnt] = entry->idx;
			++self->indexCnt;
		}
	}
}

void objVertexCacheFree(ObjVertexCache* self)
{
	{	// delete all entries in cache to free hash-table memory
		struct ObjVertexEntry *entry, *tmp;

		HASH_ITER(hh, self->cache, entry, tmp) {
			HASH_DEL(self->cache, entry);
		}
	}

	free(self->entryBuf);
	free(self->vertexBuf);
	free(self->indexBuf);
}

CrBool meshInitWithObjFile(Mesh* self, const char* path, InputStream* stream)
{
	void* fp;
	char readbuf[512];
	char* token;

	ObjBuffer vbuf  = {nullptr, sizeof(CrVec3), 128, 0};
	ObjBuffer vtbuf = {nullptr, sizeof(CrVec3), 128, 0};
	ObjBuffer vnbuf = {nullptr, sizeof(CrVec3), 128, 0};
	ObjBuffer fbuf  = {nullptr, sizeof(ObjFace), 128, 0};
	size_t vpf = 0;

	if(nullptr == (fp = stream->open(path))) {
		crDbgStr("obj file %s not found", path);
		return CrFalse;
	}

	objBufferResize(&vbuf);
	objBufferResize(&vtbuf);
	objBufferResize(&vnbuf);
	objBufferResize(&fbuf);

	//while( fgets(readbuf, 512, fp) ) {
	while( Stream_gets(stream, readbuf, 512, fp) ) {
		if('#' == readbuf[0])
			continue;

		token = strtok(readbuf, objWhitespace);

		if(nullptr == token) {
			continue;
		}
		else if(0 == strcmp(token, "v")) {
			objReadVec3((CrVec3*)objBufferAppend(&vbuf));
		}
		else if(0 == strcmp(token, "vt")) {
			objReadVec3((CrVec3*)objBufferAppend(&vtbuf));
		}
		else if(0 == strcmp(token, "vn")) {
			objReadVec3((CrVec3*)objBufferAppend(&vnbuf));
		}
		else if(0 == strcmp(token, "f")) {
			size_t vcnt = objReadFace((ObjFace*)objBufferAppend(&fbuf));
			vpf = vcnt > vpf ? vcnt : vpf;
		}
	}

	stream->close(fp);

	// flatten vertices
	/*
	meshInit(self, fbuf.cnt * vpf, fbuf.cnt * vpf);
	self->vertexPerPatch = vpf;

	{
		size_t i=0, j=0, vcnt = 0;
		CrVec3* v = (CrVec3*)self->vertex.buffer;
		CrVec3* n = (CrVec3*)self->normal.buffer;
		CrVec2* t = (CrVec2*)self->texcoord[0].buffer;
		unsigned short* id = (unsigned short*)self->index.buffer;

		for(i = 0; i < fbuf.cnt; ++i) {
			ObjFace* f = &((ObjFace*)fbuf.data)[i];
			for(j = 0; j < vpf; ++j) {
				*v = ((CrVec3*)vbuf.data)[f->vert[j].v];

				if(vnbuf.cnt) {
					*n = ((CrVec3*)vnbuf.data)[f->vert[j].n];
				}

				if(vtbuf.cnt) {
					t->x = ((CrVec3*)vtbuf.data)[f->vert[j].t].x;
					t->y = ((CrVec3*)vtbuf.data)[f->vert[j].t].y;
				}

				*id = (unsigned short)vcnt;

				++v; ++n; ++t; ++id;
				++vcnt;
			}
		}
	}

	meshCommit(self);
	*/
	// build indexed mesh
	{
		size_t i;
		CrVec3* v; CrVec3* n; CrVec2* t;
		unsigned short* id;

		ObjVertexCache vcache;
		objVertexCacheInit(&vcache, (ObjFace*)fbuf.data, fbuf.cnt, vpf);

		meshInit(self, vcache.vertexCnt, vcache.indexCnt);
		self->vertexPerPatch = vpf;

		v = (CrVec3*)self->vertex.buffer;
		n = (CrVec3*)self->normal.buffer;
		t = (CrVec2*)self->texcoord[0].buffer;
		id = (unsigned short*)self->index.buffer;

		for(i = 0; i < vcache.vertexCnt; ++i) {

			*v = ((CrVec3*)vbuf.data)[vcache.vertexBuf[i].v];

			if(vnbuf.cnt) {
				*n = ((CrVec3*)vnbuf.data)[vcache.vertexBuf[i].n];
			}

			if(vtbuf.cnt) {
				t->x = ((CrVec3*)vtbuf.data)[vcache.vertexBuf[i].t].x;
				t->y = ((CrVec3*)vtbuf.data)[vcache.vertexBuf[i].t].y;
			}

			++v; ++n; ++t;
		}

		for(i = 0; i < vcache.indexCnt; ++i) {
			*id = (unsigned short)vcache.indexBuf[i];
			++id;
		}

		objVertexCacheFree(&vcache);

		meshCommit(self);
	}
	/**/

	// clean up
	vbuf.cap = 0;
	vtbuf.cap = 0;
	vnbuf.cap = 0;
	fbuf.cap = 0;

	objBufferResize(&vbuf);
	objBufferResize(&vtbuf);
	objBufferResize(&vnbuf);
	objBufferResize(&fbuf);

	return CrTrue;
}
