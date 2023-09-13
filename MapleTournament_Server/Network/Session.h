#pragma once


// 1. Server���� ��� �ִ��� ���� (�κ�, ��, ������)
// 2.

#include <WinSock2.h>
#include <map>
#include "../Setting.h"

class User;

enum class eSessionState
{
	None,
	Login,
	Lobby,
	WatingRoom,
	InGame
};

typedef struct _unpPacket
{
	char* unprocessedPacket;
	int size;
} UnpPacket;

class Session
{
private:
	eSessionState m_eState = eSessionState::Login;
	User* m_pUser = nullptr;
	SOCKET			m_socket;

	UnpPacket m_unpPacket = {};

	static std::map<ePacketType, void(*)(Session*, char*)> m_mapPacketHandlerCallback;

public:
	Session(SOCKET _socket);
	virtual ~Session();

	void ChangeSessionState(eSessionState _eState) { m_eState = _eState; }
	eSessionState GetSessionState() const { return m_eState; }
	SOCKET GetSocket() const { return m_socket; }
	User* GetUser() const { return m_pUser; }

	void SetUser(User* _pUser) { m_pUser = _pUser; }

	void ProcessPacket(ePacketType _eType, char* _pPacket);

	void SaveUnprocessedPacket(char* _pPacket, int _totalSize);
	void LoadUnprocessedPacket(char* _pPacket, int& _totalSize);
};
	
