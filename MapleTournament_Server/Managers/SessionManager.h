#pragma once

#include <vector>
#include <winsock2.h>
#include <string>
#include <array>

#include "../Setting.h"
#include "../Defines.h"
#include "../Network/Selector.h"
#include "../Network/Session.h"

class SessionManager
{
private:
	unsigned int m_count = 0;
	std::array<Session, CLIENT_SESSION_MAX_SIZE> m_arrSession;
	std::vector<unsigned int> m_vecLeftId;
	fd_set_ex			m_fdUser;

public:
	bool Init(SOCKET _hSocketServer);

	bool RegisterSession(SOCKET _socket);
	Session* FindSession(SOCKET _socket);
	bool RemoveSession(SOCKET _socket);

	void GetVecSession(std::vector<Session*>& _vecSession);
	u_int GetLoginedUserCount() const;
	const fd_set_ex& GetFDUser() const { return m_fdUser; }

	void SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket = (SOCKET)0);

	SINGLETON(SessionManager)
};

