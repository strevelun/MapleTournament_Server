#include "ServerApp.h"
#include <WinSock2.h>

#pragma comment( lib, "ws2_32.lib")

int main()
{
	ServerApp app;
	if (app.Init("192.168.219.104", 30001))
	    app.Run();

    system("pause");
	return 0;
}