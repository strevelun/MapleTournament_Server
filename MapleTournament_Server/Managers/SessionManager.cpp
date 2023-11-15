#include "SessionManager.h"
#include "UserManager.h"
#include "../Network/User.h"

SessionManager* SessionManager::m_pInst = nullptr;

typedef unsigned short ushort;

SessionManager::SessionManager()
{
	m_vecLeftId.resize(CLIENT_SESSION_MAX_SIZE, 0);
	for (int i = 0; i < CLIENT_SESSION_MAX_SIZE; i++)
		m_vecLeftId[i] = i;
}

SessionManager::~SessionManager()
{
}

bool SessionManager::Init(SOCKET _hSocketServer)
{
	FD_ZERO(&m_fdUser);
	FD_SET_EX(_hSocketServer, &m_fdUser, nullptr);
	return true;
}

bool SessionManager::RegisterSession(SOCKET _socket)
{
	if (m_count >= CLIENT_SESSION_MAX_SIZE) return false;

	unsigned int id = m_vecLeftId.back();
	m_vecLeftId.pop_back();

	m_arrSession[id].Init();
	m_arrSession[id].SetSocket(_socket);
	m_arrSession[id].ChangeSessionState(eSessionState::Login);
	m_arrSession[id].SetId(id);
	FD_SET_EX(_socket, &m_fdUser, &m_arrSession[id]);
	m_count++;

	return true;
}

Session* SessionManager::FindSession(SOCKET _socket)
{
	for (auto& session : m_arrSession)
	{
		if (session.GetSessionState() == eSessionState::None) continue;

		if (session.GetSocket() == _socket) return &session;
	}
	return nullptr;
}

bool SessionManager::RemoveSession(SOCKET _socket)
{
	Session* pSession = FindSession(_socket);
	if (!pSession) return false;

	pSession->ChangeSessionState(eSessionState::None);
	m_vecLeftId.push_back(pSession->GetId());
	m_count--;
	FD_CLR_EX(_socket, &m_fdUser);
	closesocket(_socket);
	
	return true;
}

void SessionManager::GetVecSession(std::vector<Session*>& _vecSession)
{
	for (auto& session : m_arrSession)
	{
		if (session.GetSessionState() == eSessionState::None) continue;

		_vecSession.push_back(&session);
	}
}

u_int SessionManager::GetLoginedUserCount() const
{
	u_int loginCount = 0;
	for (const auto& session : m_arrSession)
	{
		eSessionState eState = session.GetSessionState();
		if (eState == eSessionState::None || eState == eSessionState::Login) continue;
		loginCount++;
	}
	return loginCount;
}

void SessionManager::SendAll(char* _pBuffer, eSessionState _eSessionState, SOCKET _exceptSocket)
{
	int len = (int)*(unsigned short*)_pBuffer;
	for (const auto& session : m_arrSession)
	{
		if (session.GetSocket() == _exceptSocket) continue;
		if (session.GetSessionState() != _eSessionState) continue;

		send(session.GetSocket(), _pBuffer, len, 0);
	}
}
