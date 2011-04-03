#include "Memory.h"

void* crDefaultAlloc(size_t sizeInBytes, const char* id)
{
	return realloc(nullptr, sizeInBytes);
}

void crDefaultFree(void* ptr, const char* id)
{
	realloc(ptr, 0);
}

static CrMemory _crMemory = 
{
	crDefaultAlloc,
	crDefaultFree
};

CR_API CrMemory* crMemory()
{
	return &_crMemory;
}