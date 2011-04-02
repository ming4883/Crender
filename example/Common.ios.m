#include "Common.h"
#include "../crender/Memory.h"

#include "../lib/glsw/glsw.h"

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSData.h>

typedef struct iosFILE
{
	size_t offset;
	NSData* data;
} iosFILE;

void* myOpen(const char* filename)
{
	NSString* path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String: filename] ofType:nil];

	if(nil == path)
		return nil;

	NSData* data = [[NSData dataWithContentsOfFile:path] retain];

	if(nil == data)
		return nil;

	iosFILE* file = crMemory()->alloc(sizeof(iosFILE), "io");
	file->offset = 0;
	file->data = data;
	return file;
}

void myClose(void* handle)
{
	if(nil == handle)
		return;

	iosFILE* file = handle;
	[file->data release];
	crMemory()->free(file, "io");
}

size_t myRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	//return (size_t)AAsset_read((AAsset*)handle, buff, elsize * nelem);
	iosFILE* file = handle;

	NSRange range;
	range.location = file->offset;
	range.length = elsize * nelem;

	if(range.location + range.length >= [file->data length]) {
		range.length = ([file->data length]-1) - range.location;
	}

	if(range.length > 0) {
		[file->data getBytes: buff range:range];
		file->offset += range.length;
	}

	return range.length;
}

