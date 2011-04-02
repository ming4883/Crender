#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (* glswread) (void *buff, size_t elsize, size_t nelem, void *parm);
typedef void* (* glswopen) (const char *filename);
typedef void (* glswclose) (void* handle);

typedef struct glswFileSystem {
	glswread read;
	glswopen open;
	glswclose close;
} glswFileSystem;

int glswInit(glswFileSystem* fsys);
int glswShutdown();
int glswSetPath(const char* pathPrefix, const char* pathSuffix);
const char* glswGetShader(const char* effectKey);
const char* glswGetError();
int glswAddDirectiveToken(const char* token, const char* directive);

#ifdef __cplusplus
}
#endif
