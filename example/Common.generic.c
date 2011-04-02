#include "Common.h"

#include "../lib/glsw/glsw.h"

void* myOpen(const char* filename)
{
	static char buf[512];

	static const char* paths[] = {
		"../../example/",
		"../example/",
		"../../media/",
		"../media/",
		"media",
		nullptr,
	};

	FILE* fp;
	const char** path;

	if(nullptr != (fp = fopen(filename, "rb")))
		return fp;

	for(path = paths; nullptr != *path; ++path) {
		strcpy(buf, *path);
		if(nullptr != (fp = fopen(strcat(buf, filename), "rb")))
			return fp;
	}

	crDbgStr("failed to open asset '%s'!\n", filename);

	return nullptr;
}

void myClose(void* handle)
{
	fclose((FILE*)handle);
}

size_t myRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return fread(buff, elsize, nelem, (FILE*)handle);
}
