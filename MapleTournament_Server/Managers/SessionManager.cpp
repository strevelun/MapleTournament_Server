#include "SessionManager.h"
#include "UserManager.h"
#include "../User/User.h"

SessionManager* SessionManager::m_pInst = nullptr;

typedef unsigned short ushort;

SessionManager::SessionManager()
{
	m_vecLeftId.resize(Session::ClientSessionMaxSize, 0);
	for (int i = 0; i < Session::ClientSessionMaxSize; i++)
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
	if (m_count >= Session::ClientSessionMaxSize) return false;

	unsigned int id = m_vecLeftId.back();
	m_vecLeftId.pop_back();

	m_arrSession[id].Init(_socket);
	m_arrSession[id].ChangeSessionState(eSessionState::Login);
	m_arrSession[id].SetId(id);
	FD_SET_EX(_socket, &m_fdUser, &m_arrSession[id]);
	m_count++;

	return true;
}

Session* SessionManager::FindSession(unsigned int _id)
{
	if (_id > Session::ClientSessionMaxSize - 1)		return nullptr;
	if(m_arrSession[_id].GetSessionState() == eSessionState::None) 	return nullptr;

	return &m_arrSession[_id];
}

bool SessionManager::RemoveSession(unsigned int _id)
{
	Session* pSession = FindSession(_id);
	if (!pSession) return false;

	if (pSession->GetSessionState() != eSessionState::Login)
		m_mapSessionLogin.erase(_id);

	pSession->ChangeSessionState(eSessionState::None);
	m_vecLeftId.push_back(pSession->GetId());
	m_count--;
	SOCKET socket = pSession->GetSocket();
	FD_CLR_EX(socket, &m_fdUser);
	closesocket(socket);
	
	return true;
}

void SessionManager::LoginSession(unsigned int _id)
{
	Session* pSession = FindSession(_id);
	if (!pSession) return;

	m_mapSessionLogin.insert({ _id, pSession });
}

u_int SessionManager::GetLoginedUserCount() const
{
	return (u_int)m_mapSessionLogin.size();
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
