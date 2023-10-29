#include "ServerApp.h"
#include "Network/Selector.h"
#include "Network/Session.h"
#include "Managers/SessionManager.h"
#include "Managers/GameManager.h"
#include "Network/User.h"

#include <iostream>

ServerApp::ServerApp()
{
}

ServerApp::~ServerApp()
{
	GameManager::DestroyInst();
	SessionManager::DestroyInst();
	delete m_pSelector;
	delete m_pListener;
}

bool ServerApp::Init(const char* _ip, int _port)
{
	WSADATA  wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartup error" << '\n';
		return false;
	}

	m_pListener = new TCPListener(_ip, _port);
	if (!SessionManager::GetInst()->Init(m_pListener->GetSocket())) return false;
	m_pSelector = new Selector(m_pListener->GetSocket());

	if (!m_pListener->Start(5)) return false;
	if (!GameManager::GetInst()->Init()) return false;

	return true;
}

void ServerApp::Run()
{
	printf("서버 실행 중\n");
	while (1)
	{
		m_pSelector->Select();
		GameManager::GetInst()->Update();
	}
}