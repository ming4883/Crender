#ifndef __EXAMPLE_REMOTE_IMPL_ANDROID_H__
#define __EXAMPLE_REMOTE_IMPL_ANDROID_H__

#include "Remote.impl.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RemoteConfigImpl
{
	RemoteConfigImplBase base;
	// thread
	pthread_t threadId;

} RemoteConfigImpl;

void remoteConfigInitImpl(RemoteConfigImpl* self, int port, CrBool useThread);

void remoteConfigFreeImpl(RemoteConfigImpl* self);

void remoteConfigLockImpl(RemoteConfigImpl* self);

void remoteConfigUnlockImpl(RemoteConfigImpl* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_IMPL_ANDROID_H__