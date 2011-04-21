#if defined(CR_WIN32)
#include "Remote.impl.windows.h"
#elif defined(CR_ANDROID)
#include "Remote.impl.android.h"
#elif defined(CR_APPLE_IOS)
#include "Remote.impl.ios.h"
#endif

#include <stdio.h>


typedef struct RemoteVar
{
	RemoteVarDesc desc;
	CrHashCode id;
	UT_hash_handle hh;
} RemoteVar;

RemoteVar* remoteVarAlloc()
{
	RemoteVar* self;
	self = malloc(sizeof(RemoteVar));
	memset(self, 0, sizeof(RemoteVar));
	return self;
}

void remoteVarFree(RemoteVar* self)
{
	free(self->desc.name);
	free(self);
}

void remoteVarInit(RemoteVar* self, RemoteVarDesc desc)
{
	self->id = CrHash(desc.name);
	self->desc.name = strdup(desc.name);
	self->desc.value = desc.value;
	self->desc.upperBound = desc.upperBound;
	self->desc.lowerBound = desc.lowerBound;
}

RemoteVar* remoteConfigFindVar(RemoteConfig* self, const char* name);
void remoteConfigAddVar(RemoteConfig* self, RemoteVarDesc desc);

RemoteConfig* remoteConfigAlloc()
{
	RemoteConfig* self;
	CrAllocWithImpl(self, RemoteConfig, RemoteConfigImpl);
	return self;
}

void remoteConfigIndexhtml(httpd* server)
{
	struct RemoteVar *curr, *tmp;
	RemoteConfig* self = (RemoteConfig*)server->userData;
	RemoteConfigImplBase* impl = (RemoteConfigImplBase*)self->impl;

	// html head
	{
		char* html = "\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
  <link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css' rel='stylesheet' type='text/css'/>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js'></script>\n\
  <script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js'></script>\n\
  <style type='text/css'>\n\
  #slider { margin: 10px; }\n\
  body {\n\
    font-size:75%;\n\
    font:.75em/1.5 sans-serif;\n\
    background-color:LightBlue;\n\
  }\n\
  table.var {\n\
    background-color: white;\n\
  }\n\
  table.var th {\n\
    padding: 5px;\n\
    text-align:left;\n\
  }\n\
  table.var td {\n\
    padding: 8px;\n\
  }\n\
  </style>\n\
";
		httpdOutput(server, html);
	}

	// javascripts
	HASH_ITER(hh, impl->vars, curr, tmp) {

		char* html = "\
  <script>\n\
  $(document).ready(function() {\n\
    var name = '%s';\n\
    var elem = $('#'+name);\n\
    var elemVal = $('#'+name+'_val');\n\
    elem.slider({ min:%f, max:%f, value:%f,\n\
      stop:function(event, ui) {\n\
        var data = {}; data[name] = elem.slider('value');\n\
        jQuery.get('setter', data);\n\
        elemVal.html(data[name]);\n\
      }\n\
    });\n\
    elemVal.html(elem.slider('value'));\n\
  });\n\
  </script>\n\
";
		httpdPrintf(server, html, curr->desc.name, curr->desc.lowerBound, curr->desc.upperBound, *(curr->desc.value));
	}

	// html body
	{
		char* html = "\
</head>\n\
<body><table class='var'>\n\
<tr><th>Name</th><th style='width:200px;'>Value</th><th></th></tr>\n\
";
		httpdOutput(server, html);
	}

	// <div> tags
	HASH_ITER(hh, impl->vars, curr, tmp) {
		char* html = "<tr><td>%s</td><td><div id='%s'></div></td><td><div id='%s_val'></div></td></tr>\n";
		httpdPrintf(server, html,curr->desc.name, curr->desc.name, curr->desc.name);
	}

	// footer
	{
		char* html = "</table></body>\n</html>";
		httpdOutput(server, html);
	}
}

void remoteConfigSetterhtml(httpd* server)
{
	RemoteVar* rvar;
	httpVar* hvar;
	RemoteConfig* self = (RemoteConfig*)server->userData;

	if(nullptr == (hvar = httpdGetVariableByPrefix(server, ""))) {
		httpdPrintf(server, "false");
		return;
	}

	if(nullptr == (rvar = remoteConfigFindVar(self, hvar->name))) {
		httpdPrintf(server, "false");
		return;
	}

	if(0 == sscanf(hvar->value, "%f", rvar->desc.value)) {
		httpdPrintf(server, "false");
		return;
	}
	httpdPrintf(server, "true");
}

void remoteConfigInit(RemoteConfig* self, int port, CrBool useThread)
{
	RemoteConfigImplBase* impl;

	if(nullptr == self)
		return;

	impl = (RemoteConfigImplBase*)self->impl;
	impl->http = httpdCreate(nullptr, port);
	impl->http->userData = self;
	httpdAddCContent(impl->http, "/", "index.html", HTTP_TRUE, nullptr, remoteConfigIndexhtml);
	httpdAddCContent(impl->http, "/", "setter", HTTP_FALSE, nullptr, remoteConfigSetterhtml);

	remoteConfigInitImpl(self->impl, port, useThread);
}

void remoteConfigFree(RemoteConfig* self)
{
	struct RemoteVar *curr, *tmp;
	RemoteConfigImplBase* impl;

	if(nullptr == self)
		return;

	impl = (RemoteConfigImplBase*)self->impl;

	remoteConfigFreeImpl(self->impl);

	HASH_ITER(hh, impl->vars, curr, tmp) {
		HASH_DEL(impl->vars, curr);
		remoteVarFree(curr);
	}

	httpdDestroy(impl->http);

	free(self);
}

void remoteConfigProcessRequest(RemoteConfig* self)
{
	struct timeval to;
	RemoteConfigImplBase* impl;

	if(nullptr == self)
		return;

	impl = (RemoteConfigImplBase*)self->impl;

	to.tv_sec = 0;
	to.tv_usec = 100;

	if (httpdGetConnection(impl->http, &to) <= 0)
		return;

	if(httpdReadRequest(impl->http) < 0) {
		httpdEndRequest(impl->http);
		return;
	}

	httpdProcessRequest(impl->http);

	httpdEndRequest(impl->http);
}


void remoteConfigLock(RemoteConfig* self)
{
	if(nullptr == self)
		return;

	remoteConfigLockImpl(self->impl);
}

void remoteConfigUnlock(RemoteConfig* self)
{
	if(nullptr == self)
		return;

	remoteConfigUnlockImpl(self->impl);
}

RemoteVar* remoteConfigFindVar(RemoteConfig* self, const char* name)
{
	RemoteVar* var = nullptr;
	CrHashCode id;
	RemoteConfigImplBase* impl;

	if(nullptr == self)
		return nullptr;

	impl = (RemoteConfigImplBase*)self->impl;

	id = CrHash(name);
	HASH_FIND_INT(impl->vars, &id, var);

	return var;
}

void remoteConfigAddVar(RemoteConfig* self, RemoteVarDesc desc)
{
	RemoteVar* var;
	RemoteConfigImplBase* impl;

	if(nullptr == self)
		return;

	impl = (RemoteConfigImplBase*)self->impl;

	var = remoteVarAlloc();
	remoteVarInit(var, desc);
	HASH_ADD_INT(impl->vars, id, var);
}

void remoteConfigAddVars(RemoteConfig* self, RemoteVarDesc* descs)
{
	RemoteVarDesc* curr = descs;

	while(nullptr != curr->name) {
		remoteConfigAddVar(self, *curr);
		++curr;
	}
}
