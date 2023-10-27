#include "SessionManager.h"
#include "UserManager.h"
#include "../Network/Session.h"
#include "../Network/User.h"

SessionManager* SessionManager::m_pInst = nullptr;

typedef unsigned short ushort;

SessionManager::SessionManager()
{
	m_vecSession.reserve(64);
}

SessionManager::~SessionManager()
{
	for (auto& s : m_vecSession)
	{
		delete s;
	}
}

bool SessionManager::Init()
{
	return true;
}

bool SessionManager::AddSession(Session* _pSession)
{
	if (m_vecSession.size() >= 64) return false;

	m_vecSession.push_back(_pSession);
	return true;
}

Session* SessionManager::FindSession(SOCKET _socket)
{
	size_t size = m_vecSession.size();
	for (u_int i = 0; i < size; i++)
	{
		if (m_vecSession.at(i)->GetSocket() == _socket) return m_vecSession[i];
	}
	return nullptr;
}

bool SessionManager::RemoveSession(SOCKET _socket)
{
	std::vector<Session*>::iterator iter = m_vecSession.begin();
	std::vector<Session*>::iterator iterEnd = m_vecSession.end();

	for (; iter != iterEnd; iter++)
	{
		if ((*iter)->GetSocket() == _socket)
		{
			delete* iter;
			m_vecSession.erase(iter);
			return true;
		}
	}
	return false;
}

void SessionManager::SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket)
{
	int len = (int)*(unsigned short*)_pBuffer;
	for (auto& session : m_vecSession)
	{
		if (session->GetSocket() == _exceptSocket) continue;
		if (session->GetSessionState() != _eSessionState) continue;
		send(session->GetSocket(), _pBuffer, len, 0);
	}
}
