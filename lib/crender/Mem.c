#include "Mem.h"

void* crDefaultAlloc(size_t sizeInBytes, const char* id)
{
	return realloc(nullptr, sizeInBytes);
}

void crDefaultFree(void* ptr, const char* id)
{
    free(ptr);
}

static CrMem _crMemory = 
{
	crDefaultAlloc,
	crDefaultFree
};

CR_API CrMem* crMem()
{
	return &_crMemory;
}