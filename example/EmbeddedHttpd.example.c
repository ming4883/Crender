#include "Common.h"
#include "Remote.h"

/*
#include <sys/socket.h> // socket, bind, listen...
#include <arpa/inet.h>	// sockaddr_in
int serverSock;
struct sockaddr_in serverAddr;

CrBool crAppInitialize()
{
	// remember to add <uses-permission android:name="android.permission.INTERNET"></uses-permission>
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		crDbgStr("Failed to open server socket");
	}
	
	memset(&serverAddr, 0, sizeof(serverAddr));		// Clear struct
	serverAddr.sin_family = AF_INET;					// Internet/IP
	serverAddr.sin_addr.s_addr = INADDR_ANY;			// Incoming addr
	serverAddr.sin_port = htons(8080);					// server port
	
	if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		crDbgStr("Failed to bind the server socket");
	}
    
	// Listen on the server socket
	if (listen(serverSock, SOMAXCONN) < 0) {
		crDbgStr("Failed to listen on server socket");
	}
	
	crRenderTargetSetViewport(0, 0, (float)crAppContext.xres, (float)crAppContext.yres, -1, 1);
	
	crDbgStr("server started");

	return CrTrue;
}

void crAppFinalize()
{
	close(serverSock);
}

#define BUFFSIZE 512

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	int sock;
	struct sockaddr_in clientAddr;
	
	unsigned int clientlen = sizeof(clientAddr);
	// Wait for client connection
	if ((sock = accept(serverSock, (struct sockaddr *) &clientAddr, &clientlen)) < 0) {
		//Die("Failed to accept client connection");
		return;
	}
	
	{
		char buffer[BUFFSIZE];
		int received = -1;
		// Receive message
		if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
			crDbgStr("Failed to receive initial bytes from client");
		}
		crDbgStr("received %d %s", received, buffer);
		// Send bytes and check for more incoming data in loop
		while (received > 0) {
		  // Send back received data
		  if (send(sock, buffer, received, 0) != received) {
			crDbgStr("Failed to send bytes to client");
		  }
		  crDbgStr("reply sent");
		}
		close(sock);
	}
}

void crAppHandleMouse(int x, int y, int action)
{
}

void crAppRender()
{
	crContextClearColor(crContext(), 0, 1, 0, 1.0f);
}

void crAppConfig()
{
	crAppContext.appName = "Embedded Httpd";
	crAppContext.xres = 480;
	crAppContext.yres = 320;
	crAppContext.multiSampling = CrFalse;
	crAppContext.vsync = CrFalse;
	crAppContext.apiMajorVer = 2;
	crAppContext.apiMinorVer = 1;
}
*/

RemoteConfig* _config = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void crAppUpdate(unsigned int elapsedMilliseconds)
{
	//remoteConfigProcessRequest(_config);
}

void crAppHandleMouse(int x, int y, int action)
{
}

void crAppRender()
{
	float r, g, b;
	
	remoteConfigLock(_config);
	r = bgR / 255; g = bgG / 255; b = bgB / 255;
	remoteConfigUnlock(_config);

	crContextClearColor(crContext(), r, g, b, 1.0f);
}

void crAppConfig()
{
	crAppContext.appName = "Embedded Httpd";
	crAppContext.context->xres = 480;
	crAppContext.context->yres = 320;
}

void crAppFinalize()
{
	remoteConfigFree(_config);
}

CrBool crAppInitialize()
{
	RemoteVarDesc descs[] = {
		{"bgR", &bgR, 0, 255},
		{"bgG", &bgG, 0, 255},
		{"bgB", &bgB, 0, 255},
		{nullptr, nullptr, 0, 0}
	};

	crContextSetViewport(crContext(), 0, 0, (float)crContext()->xres, (float)crContext()->yres, -1, 1);

	_config = remoteConfigAlloc();
	remoteConfigInit(_config, 8080, CrTrue);
	remoteConfigAddVars(_config, descs);

	return CrTrue;
}