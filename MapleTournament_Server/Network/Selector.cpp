#include "Selector.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Network/Session.h"
#include "User.h"

Selector::Selector(SOCKET _hSocketServer)
	: m_hSocketServer(_hSocketServer)
{
	FD_ZERO(&m_fdReads);
	FD_SET_EX(m_hSocketServer, &m_fdUser, nullptr);

	m_vecClientSocket.reserve(64);
}

Selector::~Selector()
{
}

const std::vector<Session*>& Selector::Select()
{
	SOCKET clientSocket;
	if(!m_vecClientSocket.empty())
		m_vecClientSocket.clear();

	m_fdReads = m_fdUser;
	int	iRet = select(0, &m_fdReads, 0, 0, 0);
	if (iRet == SOCKET_ERROR) throw L"select returns SOCKET_ERROR in Selector::Select()";

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
				m_vecClientSocket.push_back(m_fdUser.fd_array_session[i]);
			}
		}
	}
	return m_vecClientSocket;
}
