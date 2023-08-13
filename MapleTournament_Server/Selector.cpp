#include "Selector.h"
#include "packet/ConnectPacket.h"
#include "UserManager.h"
#include "User.h"

Selector::Selector(SOCKET _hSocketServer)
	: m_hSocketServer(_hSocketServer)
{
	FD_ZERO(&m_fdSocketInfos);
	FD_ZERO(&m_fdReads);
	FD_SET(m_hSocketServer, &m_fdSocketInfos);
}

Selector::~Selector()
{
}

std::list<SOCKET>& Selector::Select() // 누구를 select했는지 리스트 리턴
{
	SOCKET clientSocket;

	m_fdReads = m_fdSocketInfos;
	int	iRet = select(0, &m_fdReads, 0, 0, 0);
	if (iRet == SOCKET_ERROR) throw L"select returns SOCKET_ERROR in Selector::Select()";

	for (u_int i = 0; i < m_fdSocketInfos.fd_count; i++)
	{
		clientSocket = m_fdSocketInfos.fd_array[i];
		if (FD_ISSET(clientSocket, &m_fdReads))
		{
			if (clientSocket == m_hSocketServer)
			{
				SOCKADDR_IN			addrClient;
				int addrSize = sizeof(addrClient);
				SOCKET acceptedClientSocket = accept(m_hSocketServer, (SOCKADDR*)&addrClient, &addrSize);
				printf("%d 연결됨\n", (int)acceptedClientSocket);
				FD_SET(acceptedClientSocket, &m_fdSocketInfos);

				ushort id = (ushort)acceptedClientSocket;
				ConnectPacket* pPacket = new ConnectPacket(id);
				UserManager::GetInst()->AddUser(id, new User(id));
				send(acceptedClientSocket, pPacket->GetPacketBuffer(), pPacket->GetPacketSize(), 0);
				delete pPacket;
			}
			else
			{
				m_clientSocketList.push_back(clientSocket);
			}
		}
	}
	return m_clientSocketList;
}
