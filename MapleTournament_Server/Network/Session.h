#pragma once


// 1. Server에서 어디에 있는지 상태 (로비, 방, 게임중)
// 2.

#include <WinSock2.h>
#include <map>
#include "../Setting.h"

class User;
class Room;



class Session
{
private:
	class Packet
	{
	private:
		friend class Session;

	private:
		static constexpr int LeastSize = sizeof(u_short) + sizeof(u_short);
		static constexpr int BufferSize = 255;

	private:
		char buffer[BufferSize];
		char tempBuffer[BufferSize];
		int size = 0;
		int startPos = 0;
		int endPos = 0;

	private:
		void MoveRight(int _byteCount);
	};

private:
	unsigned int m_id;
	eSessionState m_eState = eSessionState::Login;
	User* m_pUser = nullptr;
	Room* m_pRoom = nullptr;
	SOCKET			m_socket;

	Packet m_packet;

	static std::map<ePacketType, void(*)(Session*, char*)> m_mapPacketHandlerCallback;

public:
	Session(unsigned int _id, SOCKET _socket);
	virtual ~Session();

	void ChangeSessionState(eSessionState _eState) { m_eState = _eState; }
	eSessionState GetSessionState() const { return m_eState; }
	SOCKET GetSocket() const { return m_socket; }
	User* GetUser() const { return m_pUser; }
	Room* GetRoom() const { return m_pRoom; }
	unsigned int GetId() const { return m_id; }

	void SetUser(User* _pUser) { m_pUser = _pUser; }
	void SetRoom(Room* _pRoom) { m_pRoom = _pRoom; }

	void ProcessPacket();
	int ReceivePacket();
};
	
