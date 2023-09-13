#include "ServerApp.h"
#include <WinSock2.h>

#pragma comment( lib, "ws2_32.lib")

int main()
{
	ServerApp app;
	app.Init("192.168.219.167", 30001);
	app.Run();

	return 0;
}