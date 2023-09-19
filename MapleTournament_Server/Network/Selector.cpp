#include "Selector.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Network/Session.h"
#include "User.h"

Selector::Selector(SOCKET _hSocketServer)
	: m_hSocketServer(_hSocketServer)
{
	FD_ZERO(&m_fdReads);
	FD_ZERO(&m_fdUser);
	FD_SET_EX(m_hSocketServer, &m_fdUser, nullptr);
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

	m_fdReads = m_fdUser;
	int	iRet = select(0, &m_fdReads, 0, 0, 0);
	if (iRet == SOCKET_ERROR) return;

	for (u_int i = 0; i < m_fdUser.fd_count; i++)
	{
		clientSocket = m_fdUser.fd_array[i];
		if (FD_ISSET(clientSocket, &m_fdReads))
		{
			if (clientSocket == m_hSocketServer)
			{
				SOCKADDR_IN			addrClient;
				int addrSize = sizeof(addrClient);
				SOCKET acceptedClientSocket = accept(m_hSocketServer, (SOCKADDR*)&addrClient, &addrSize);
				printf("%d, (%d.%d.%d.%d) %d ¿¬°áµÊ\n", (int)acceptedClientSocket, addrClient.sin_addr.S_un.S_un_b.s_b1, addrClient.sin_addr.S_un.S_un_b.s_b2, addrClient.sin_addr.S_un.S_un_b.s_b3, addrClient.sin_addr.S_un.S_un_b.s_b4, addrClient.sin_port);
				
				Session* pSession = new Session(acceptedClientSocket);
				FD_SET_EX(acceptedClientSocket, &m_fdUser, pSession);
				SessionManager::GetInst()->AddSession(pSession);
			}
			else
			{
				Session* pSession = m_fdUser.fd_array_session[i];

				pSession->LoadUnprocessedPacket(recvBuffer, totalSize);

				recvSize = recv(clientSocket, recvBuffer + totalSize, sizeof(recvBuffer) - totalSize, 0);
				if (recvSize == SOCKET_ERROR)
				{
					char buffer[255];
					u_short count = sizeof(u_short);
					*(u_short*)(buffer + count) = (u_short)ePacketType::S_Exit;		count += sizeof(u_short);
					User* pUser = pSession->GetUser();
					const wchar_t* str = pUser->GetNickname();
					memcpy(buffer + count, str, wcslen(str) * 2);			                    count += (u_short)wcslen(str) * 2;
					*(wchar_t*)(buffer + count) = L'\0';								        count += 2;
					*(u_short*)buffer = count;
					SessionManager::GetInst()->SendAll(buffer, eSessionState::Lobby);

					RemoveSocket(clientSocket);
					SessionManager::GetInst()->RemoveSession(clientSocket);
					closesocket(clientSocket);
					return;
				}

				totalSize += recvSize;
				char* temp;
				u_short type;
				while (totalSize >= sizeof(u_short))
				{
					packetSize = *(u_short*)recvBuffer;
					if (packetSize > totalSize)
					{
						pSession->SaveUnprocessedPacket(recvBuffer, totalSize);
						break;
					}

					temp = recvBuffer;									temp += sizeof(u_short);
					type = *(u_short*)temp;							//temp += sizeof(u_short);

					pSession->ProcessPacket((ePacketType)type, temp);

					totalSize -= packetSize;
					memcpy(recvBuffer, recvBuffer + packetSize, totalSize);
				}
			}
		}
	}
}
