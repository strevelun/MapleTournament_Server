#include "Selector.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Network/Session.h"
#include "User.h"

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
	int					recvSize, totalSize = 0;
	u_short				packetSize = 0;
	char				recvBuffer[255];

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
				if (m_fdReads.fd_count >= FD_SETSIZE)
				{
					printf("현재 서버소켓 포함 총 %d개이기 때문에 접속 거부됨. (최대 64개)\n", m_fdReads.fd_count);
					continue;
				}

				SOCKADDR_IN			addrClient;
				int addrSize = sizeof(addrClient);
				SOCKET acceptedClientSocket = accept(m_hSocketServer, (SOCKADDR*)&addrClient, &addrSize);
				printf("%d, (%d.%d.%d.%d) %d 연결됨\n", (int)acceptedClientSocket, addrClient.sin_addr.S_un.S_un_b.s_b1, addrClient.sin_addr.S_un.S_un_b.s_b2, addrClient.sin_addr.S_un.S_un_b.s_b3, addrClient.sin_addr.S_un.S_un_b.s_b4, addrClient.sin_port);
				
				SessionManager::GetInst()->AddSession(acceptedClientSocket);
			}
			else
			{
				Session* pSession = _fdUser.fd_array_session[i];

				pSession->LoadUnprocessedPacket(recvBuffer, totalSize);

				recvSize = recv(clientSocket, recvBuffer + totalSize, sizeof(recvBuffer) - totalSize, 0);
				if (recvSize == SOCKET_ERROR)
				{
					SessionManager::GetInst()->RemoveSession(clientSocket);
					return;
				}

				totalSize += recvSize;

				while (totalSize >= 1)
				{
					packetSize = *(u_short*)recvBuffer;
					if (totalSize == 1 || packetSize > totalSize)
					{
						pSession->SaveUnprocessedPacket(recvBuffer, totalSize);
						break;
					}
					if (packetSize < sizeof(u_short) + sizeof(u_short)) 
						break;

					char* temp = recvBuffer;									temp += sizeof(u_short);
					u_short type = *(u_short*)temp;								temp += sizeof(u_short);

					pSession->ProcessPacket((ePacketType)type, temp);

					totalSize -= packetSize;
					memcpy(recvBuffer, recvBuffer + packetSize, totalSize);
				}
			}
		}
	}
}
