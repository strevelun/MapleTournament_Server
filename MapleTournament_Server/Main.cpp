#include "ServerApp.h"
#include "Setting.h"

#pragma comment( lib, "ws2_32.lib")

int main()
{

	try {
		ServerApp app("192.168.219.167", 30001);
		app.Run();
	}
	catch (const std::wstring& e)
	{
		std::cout << e.c_str() << '\n';
		return -1;
	}
	return 0;
}