#include "Remote.impl.android.h"

void* remoteConfigThread(void* param)
{
	RemoteConfigImpl* self = (RemoteConfigImpl*)param;

	while(!self->base.requestExit) {
		struct timeval to = {0, 1000};

		if (httpdGetConnection(self->base.http, &to) <= 0)
			continue;

		if(httpdReadRequest(self->base.http) < 0) {
			httpdEndRequest(self->base.http);
			continue;
		}

		remoteConfigLockImpl(self);

		httpdProcessRequest(self->base.http);

		remoteConfigUnlockImpl(self);

		httpdEndRequest(self->base.http);
	}
	return nullptr;
}

void remoteConfigInitImpl(RemoteConfigImpl* self, int port, CrBool useThread)
{
	if(useThread) {
		//InitializeCriticalSection(&self->criticalSection);
		//self->threadHandle = CreateThread(nullptr, 0, remoteConfigThread, self, 0, &self->threadId);
		pthread_create(&self->threadId, nullptr, remoteConfigThread, self);
	}
}

void remoteConfigFreeImpl(RemoteConfigImpl* self)
{
	if(nullptr != self->threadId) {
		// notify the thread to exit
		self->base.requestExit = CrTrue;

		// cleanup thread related resources
		//WaitForMultipleObjects(1, &self->threadHandle, TRUE, INFINITE);
		//CloseHandle(self->threadHandle);
		//DeleteCriticalSection(&self->criticalSection);
	}
}

void remoteConfigLockImpl(RemoteConfigImpl* self)
{	
	if(nullptr == self->threadId)
		return;

	//EnterCriticalSection(&self->criticalSection);
}

void remoteConfigUnlockImpl(RemoteConfigImpl* self)
{
	if(nullptr == self->threadId)
		return;

	//LeaveCriticalSection(&self->criticalSection);
}
