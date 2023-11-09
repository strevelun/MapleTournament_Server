#include "ServerApp.h"
#include <WinSock2.h>

#pragma comment( lib, "ws2_32.lib")
/*
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch (CEvent)
    {
    case CTRL_CLOSE_EVENT:

            return TRUE;
    }
    return FALSE;
}
*/

int main()
{
   // SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);

	ServerApp app;
	if (app.Init("192.168.219.167", 30001))
	    app.Run();

    system("pause");
	return 0;
}