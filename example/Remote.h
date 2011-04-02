#ifndef __EXAMPLE_REMOTE_H__
#define __EXAMPLE_REMOTE_H__

#include "../lib/crender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RemoteVarDesc
{
	char* name;
	float* value;
	float lowerBound, upperBound;
} RemoteVarDesc;

struct RemoteConfigImpl;

typedef struct RemoteConfig
{
	struct RemoteConfigImpl* impl;
} RemoteConfig;

RemoteConfig* remoteConfigAlloc();

void remoteConfigFree(RemoteConfig* self);

void remoteConfigInit(RemoteConfig* self, int port, CrBool useThread);

void remoteConfigAddVars(RemoteConfig* self, RemoteVarDesc* descs);

void remoteConfigProcessRequest(RemoteConfig* self);

void remoteConfigLock(RemoteConfig* self);

void remoteConfigUnlock(RemoteConfig* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_H__