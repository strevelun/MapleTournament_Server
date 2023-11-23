#pragma once

#include <WinSock2.h>
#include <map>
#include "../Setting.h"

class User;
class Room;

class Session
{
public:
	static constexpr int ClientSessionMaxSize = 63;

private:
	class Buffer
	{
	public:
		//static constexpr int HeaderSize = sizeof(u_short) + sizeof(u_short);

		static constexpr int LeastSize = sizeof(u_short) + sizeof(u_short);
		static constexpr int BufferSize = 255;

	private:
		char buffer[BufferSize];
		char tempBuffer[BufferSize];
		int size = 0;
		int startPos = 0;
		int endPos = 0;

	public:
		void Init();
		u_short GetPacketSize() const { return *(u_short*)(buffer + startPos);}
		int GetRecvSize() const { return size; }
		void CheckBufferSizeReadable();
		int Recv(SOCKET _socket);
		char* GetPacketStartPos();
	};

private:
	unsigned int m_id = 0;
	eSessionState m_eState = eSessionState::None;
	User* m_pUser = nullptr;
	Room* m_pRoom = nullptr;
	SOCKET			m_socket;

	Buffer m_buffer;

	static std::map<ePacketType, void(*)(Session*, char*)> m_mapPacketHandlerCallback;

public:
	Session();
	~Session();

	void Init(SOCKET _socket);

	void ChangeSessionState(eSessionState _eState) { m_eState = _eState; }
	eSessionState GetSessionState() const { return m_eState; }
	SOCKET GetSocket() const { return m_socket; }
	User* GetUser() const { return m_pUser; }
	Room* GetRoom() const { return m_pRoom; }
	unsigned int GetId() const { return m_id; }

	void SetId(unsigned int _id) { m_id = _id; }
	void SetSocket(SOCKET _socket) { m_socket = _socket; }
	void SetUser(User* _pUser) { m_pUser = _pUser; }
	void SetRoom(Room* _pRoom) { m_pRoom = _pRoom; }

	void ProcessPacket();
	int ReceivePacket();
};
	
