#ifndef __EXAMPLE_REMOTE_IMPL_H__
#define __EXAMPLE_REMOTE_IMPL_H__

#include "Remote.h"

#include <stdio.h>
#include "../lib/crender/StrHash.h"
#include "../lib/crender/uthash/uthash.h"
#include "../lib/httpd/httpd.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RemoteVar;

typedef struct RemoteConfigImplBase
{
	struct RemoteVar* vars;
	httpd* http;
	CrBool requestExit;
	
} RemoteConfigImplBase;

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_REMOTE_IMPL_H__