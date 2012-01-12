#include "Mem.h"

void* crDefaultAlloc(size_t sizeInBytes, const char* id)
{
	return realloc(nullptr, sizeInBytes);
}

void crDefaultFree(void* ptr, const char* id)
{
    if(nullptr != ptr)
        realloc(ptr, 0);
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