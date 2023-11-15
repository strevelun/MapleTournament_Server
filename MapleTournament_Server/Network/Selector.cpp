#include "Selector.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Network/Session.h"
#include "../User/User.h"

Selector::Selector(SOCKET _hSocketServer)
	: m_hSocketServer(_hSocketServer)
{
	FD_ZERO(&m_fdReads);
	//FD_SET_EX(m_hSocketServer, &m_fdUser, nullptr);
}

Selector::~Selector()
{
}

void Selector::Select()
{
	SOCKET clientSocket;
	int packetLeastSize = sizeof(u_short) + sizeof(u_short);
	int					recvSize = 0, totalSize = 0;
	u_short				packetSize = 0;

	timeval timeout = { 0, 0 };

	//m_fdReads = m_fdUser;
	const fd_set_ex& _fdUser = SessionManager::GetInst()->GetFDUser();
	m_fdReads = _fdUser;
	int	iRet = select(0, &m_fdReads, 0, 0, &timeout);
	if (iRet == 0) return;
	if (iRet == SOCKET_ERROR) return;

	for (u_int i = 0; i < _fdUser.fd_count; i++)
	{
		clientSocket = _fdUser.fd_array[i];
		if (FD_ISSET(clientSocket, &m_fdReads))
		{
			if (clientSocket == m_hSocketServer)
			{
				SOCKADDR_IN			addrClient;
				int addrSize = sizeof(addrClient);
				SOCKET acceptedClientSocket = accept(m_hSocketServer, (SOCKADDR*)&addrClient, &addrSize);
				printf("%d, (%d.%d.%d.%d) %d 연결됨\n", (int)acceptedClientSocket, addrClient.sin_addr.S_un.S_un_b.s_b1, addrClient.sin_addr.S_un.S_un_b.s_b2, addrClient.sin_addr.S_un.S_un_b.s_b3, addrClient.sin_addr.S_un.S_un_b.s_b4, addrClient.sin_port);
				
				if (!SessionManager::GetInst()->RegisterSession(acceptedClientSocket))
				{
					printf("현재 서버소켓 포함 총 64개이기 때문에 접속 거부됨. (최대 64개)\n");
					closesocket(acceptedClientSocket);
					printf("%d 로그아웃 됨\n", (int)acceptedClientSocket);
				}
			}
			else
			{
				Session* pSession = _fdUser.fd_array_session[i];

				int result = pSession->ReceivePacket();
				if(result == SOCKET_ERROR || result == 0)
					SessionManager::GetInst()->RemoveSession(clientSocket);

				pSession->ProcessPacket();
			}
		}
	}
}
