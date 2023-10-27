#pragma once

#include <vector>
#include <winsock2.h>
#include <string>

#include "../Setting.h"
#include "../Defines.h"

class Session;

class SessionManager
{
private:
	std::vector<Session*> m_vecSession; // std::array

public:
	bool Init();

	bool AddSession(Session* _pSession);
	Session* FindSession(SOCKET _socket);
	bool RemoveSession(SOCKET _socket);

	const std::vector<Session*>& GetVecSession() const { return m_vecSession; }

	void SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket = (SOCKET)0);

	SINGLETON(SessionManager)
};

