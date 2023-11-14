#include "ServerApp.h"
#include <WinSock2.h>

#include <ctime>

#pragma comment( lib, "ws2_32.lib")

int main()
{
    srand((unsigned int)time(0));

	ServerApp app;
	if (app.Init("192.168.219.167", 30001))
	    app.Run();

    system("pause");
	return 0;
}