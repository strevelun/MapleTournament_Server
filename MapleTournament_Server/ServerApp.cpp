#include "ServerApp.h"
#include "TCPNetwork.h"

ServerApp::ServerApp(const char* _serverIP, int _serverPort)
{
	m_pNetwork = new TCPNetwork(_serverIP, _serverPort);
	m_pNetwork->Start();
}

ServerApp::~ServerApp()
{
	delete m_pNetwork;
}

void ServerApp::Run()
{
	printf("¼­¹ö run\n");
	while (1)
	{
		if (!m_pNetwork->Update()) break;

	}
}
