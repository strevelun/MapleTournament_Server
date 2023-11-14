#pragma once

#include <vector>
#include <winsock2.h>
#include <string>

#include "../Setting.h"
#include "../Defines.h"
#include "../Network/Selector.h"

class Session;

class SessionManager
{
private:
	int m_sessionId = 0;
	std::vector<Session*> m_vecSession; 
	fd_set_ex			m_fdUser;

public:
	bool Init(SOCKET _hSocketServer);

	bool AddSession(SOCKET _socket);
	Session* FindSession(SOCKET _socket);
	bool RemoveSession(SOCKET _socket);

	const std::vector<Session*>& GetVecSession() const { return m_vecSession; }
	u_int GetLoginedUserCount() const;
	const fd_set_ex& GetFDUser() const { return m_fdUser; }

	void SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket = (SOCKET)0);

	SINGLETON(SessionManager)
};

