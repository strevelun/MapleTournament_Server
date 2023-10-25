#pragma once

#include <vector>
#include <winsock2.h>
#include <string>

#include "../Setting.h"

class Session;

class SessionManager
{
private:
	static SessionManager* m_inst;

	std::vector<Session*> m_vecSession; // std::array

	SessionManager();
	~SessionManager();

public:
	static SessionManager* GetInst()
	{
		if (!m_inst)
			m_inst = new SessionManager;
		return m_inst;
	}

	static void DestroyInst()
	{
		if (m_inst)
			delete m_inst;
		m_inst = nullptr;
	}

	bool Init();

	bool AddSession(Session* _pSession);
	Session* FindSession(SOCKET _socket);
	bool RemoveSession(SOCKET _socket);

	const std::vector<Session*>& GetVecSession() const { return m_vecSession; }

	void SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket = (SOCKET)0);
};

