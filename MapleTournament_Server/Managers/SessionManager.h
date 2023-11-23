#pragma once

#include <vector>
#include <winsock2.h>
#include <string>
#include <array>
#include <map>

#include "../Setting.h"
#include "../Defines.h"
#include "../Network/Selector.h"
#include "../Network/Session.h"

class SessionManager
{
private:
	unsigned int m_count = 0;

	std::map<unsigned int, Session*> m_mapSessionLogin;
	std::array<Session, Session::ClientSessionMaxSize> m_arrSession;
	std::vector<unsigned int> m_vecLeftId;
	fd_set_ex			m_fdUser;

public:
	bool Init(SOCKET _hSocketServer);

	bool RegisterSession(SOCKET _socket);
	Session* FindSession(unsigned int _id);
	bool RemoveSession(unsigned int _id);
	void LoginSession(unsigned int _id);

	const std::map<unsigned int, Session*>& GetMapLoginSession() const { return m_mapSessionLogin; }
	u_int GetLoginedUserCount() const;
	const fd_set_ex& GetFDUser() const { return m_fdUser; }

	void SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket = (SOCKET)0);

	SINGLETON(SessionManager)
};

