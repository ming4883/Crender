#ifndef __EXAMPLE_REMOTE_IMPL_WINDOWS_H__
#define __EXAMPLE_REMOTE_IMPL_WINDOWS_H__

#include "Remote.impl.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RemoteConfigImpl
{
	RemoteConfigImplBase base;
	// thread
	DWORD threadId;
	HANDLE threadHandle;
	CRITICAL_SECTION criticalSection;

} RemoteConfigImpl;

void remoteConfigInitImpl(RemoteConfigImpl* self, int port, CrBool useThread);

void remoteConfigFreeImpl(RemoteConfigImpl* self);

void remoteConfigLockImpl(RemoteConfigImpl* self);

void remoteConfigUnlockImpl(RemoteConfigImpl* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_IMPL_WINDOWS_H__