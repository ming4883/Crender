#include "Common.h"

#include "../lib/glsw/glsw.h"

#include <android_native_app_glue.h>
#include <android/asset_manager.h>

extern struct android_app* CR_ANDROID_APP;

void* myOpen(const char* filename)
{
	AAsset* asset = AAssetManager_open(CR_ANDROID_APP->activity->assetManager, filename, AASSET_MODE_UNKNOWN);

	if(nullptr == asset) {
		crDbgStr("failed to open asset '%s'!\n", filename);
	}
	return asset;
}

void myClose(void* handle)
{
	AAsset_close((AAsset*)handle);
}

size_t myRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return (size_t)AAsset_read((AAsset*)handle, buff, elsize * nelem);
}

