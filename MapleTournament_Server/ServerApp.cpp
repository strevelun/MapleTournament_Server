#include "ServerApp.h"
#include "Network/Selector.h"
#include "Network/Session.h"
#include "Managers/SessionManager.h"
#include "Network/User.h"

#include <iostream>

ServerApp::ServerApp()
{
}

ServerApp::~ServerApp()
{
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
	m_pSelector = new Selector(m_pListener->GetSocket());

	if (!m_pListener->Start(5)) return false;
	if (!SessionManager::GetInst()->Init()) return false;

	return true;
}

void ServerApp::Run()
{
	printf("서버 실행 중\n");
	while (1)
	{
		const std::vector<Session*>& clientSocketList = m_pSelector->Select();
		size_t size = clientSocketList.size();
		for (u_int i = 0; i < size; i++)
		{
			ReceivePacket(clientSocketList[i]);
		}
	}
}

void ServerApp::ReceivePacket(Session* _pSession)
{
	int					recvSize,  totalSize = 0;
	u_short				packetSize = 0;
	char				recvBuffer[255];
	SOCKET				clientSocket = _pSession->GetSocket();

	_pSession->LoadUnprocessedPacket(recvBuffer, totalSize);

	recvSize = recv(clientSocket, recvBuffer + totalSize, sizeof(recvBuffer) - totalSize, 0);
	if (recvSize == SOCKET_ERROR)
	{
		const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
		u_int size = vecSession.size();
		for (int i = 0; i < size; i++)
		{
			eSessionState eState = vecSession[i]->GetSessionState();
			if (eState != eSessionState::Lobby && eState != eSessionState::WatingRoom)
				continue;

			char buffer[255];
			u_short count = sizeof(u_short);
			*(u_short*)(buffer + count) = (u_short)ePacketType::S_Exit;		count += sizeof(u_short);
			User* pUser = _pSession->GetUser();
			const wchar_t* str = pUser->GetNickname();
			memcpy(buffer + count, str, wcslen(str) * 2);			                    count += (u_short)wcslen(str) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								        count += 2;
			*(u_short*)buffer = count;
			SessionManager::GetInst()->SendAll(buffer);
		}
		
		m_pSelector->RemoveSocket(clientSocket);
		SessionManager::GetInst()->RemoveSession(clientSocket);
		closesocket(clientSocket);
		return;
	}

	totalSize += recvSize;

	while (totalSize >= sizeof(u_short))
	{
		packetSize = *(u_short*)recvBuffer;
		if (packetSize > totalSize)
		{
			_pSession->SaveUnprocessedPacket(recvBuffer, totalSize);
			break;
		}

		char* temp = recvBuffer;								temp += sizeof(u_short);
		u_short type = *(u_short*)temp;							temp += sizeof(u_short);

		_pSession->ProcessPacket((ePacketType)type, temp);

		totalSize -= packetSize;
		memcpy(recvBuffer, recvBuffer + packetSize, totalSize);
	}
}
