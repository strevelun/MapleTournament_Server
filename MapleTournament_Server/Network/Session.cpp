#include "Session.h"
#include "../Packet/PacketHandler.h"

#include <WinSock2.h>

std::map<ePacketType, void(*)(Session*, char*)> Session::m_mapPacketHandlerCallback = {
	{ ePacketType::C_OKLogin, PacketHandler::C_OKLogin },
	{ ePacketType::C_Exit, PacketHandler::C_Exit },
	{ ePacketType::C_CreateRoom, PacketHandler::C_CreateRoom },
	{ ePacketType::C_Chat, PacketHandler::C_Chat },
	{ ePacketType::C_JoinRoom, PacketHandler::C_JoinRoom },
	{ ePacketType::C_LeaveRoom, PacketHandler::C_LeaveRoom },
	{ ePacketType::C_CheckRoomReady, PacketHandler::C_CheckRoomReady },
	{ ePacketType::C_UserRoomReady, PacketHandler::C_UserRoomReady },
	{ ePacketType::C_InGameReady, PacketHandler::C_InGameReady },
	{ ePacketType::C_UpdateUserListPage, PacketHandler::C_UpdateUserListPage },
	{ ePacketType::C_UpdateRoomListPage, PacketHandler::C_UpdateRoomListPage },
	{ ePacketType::C_UpdateUserSlot, PacketHandler::C_UpdateUserSlot },
	{ ePacketType::C_Skill, PacketHandler::C_Skill },
	{ ePacketType::C_NextTurn, PacketHandler::C_NextTurn },
	{ ePacketType::C_GameOver, PacketHandler::C_GameOver },
	{ ePacketType::C_LobbyInit, PacketHandler::C_LobbyInit },
	{ ePacketType::C_Standby, PacketHandler::C_Standby },
	{ ePacketType::C_CheckHit, PacketHandler::C_CheckHit },
	{ ePacketType::C_CheckHeal, PacketHandler::C_CheckHeal },
	{ ePacketType::C_ExitInGame, PacketHandler::C_ExitInGame },
};

Session::Session()
{

}

Session::~Session()
{
}

void Session::Init()
{
	m_packet.Init();
	m_pUser = nullptr;
	m_pRoom = nullptr;
	m_socket = 0;
}

void Session::ProcessPacket()
{
	int packetSize = 0;
	u_short type; 

	while (m_packet.size >= 2)
	{
		packetSize = *(u_short*)(m_packet.buffer + m_packet.startPos);

		if (packetSize > m_packet.size) break;
		if (packetSize < Packet::LeastSize) break;

		if (m_packet.startPos + packetSize > sizeof(m_packet.buffer))
		{
			int firstPartSize = sizeof(m_packet.buffer) - m_packet.startPos;
			memcpy(m_packet.tempBuffer, m_packet.buffer + m_packet.startPos, firstPartSize);
			memcpy(m_packet.tempBuffer + firstPartSize, m_packet.buffer, packetSize - firstPartSize);
			char* temp = m_packet.tempBuffer;				temp += sizeof(u_short);
			type = *(u_short*)temp;							temp += sizeof(u_short);
			m_mapPacketHandlerCallback[(ePacketType)type](this, temp);
		}
		else
		{
			char* temp = m_packet.buffer + m_packet.startPos;					temp += sizeof(u_short);
			type = *(u_short*)temp;												temp += sizeof(u_short);

			m_mapPacketHandlerCallback[(ePacketType)type](this, temp);
		}

		if ((ePacketType)type == ePacketType::C_Exit) return;

		m_packet.size -= packetSize;
		m_packet.startPos = (m_packet.startPos + packetSize) % sizeof(m_packet.buffer);

		if (m_packet.startPos >= Packet::BufferSize - 1)
			m_packet.MoveRight(1);

		//printf("%d바이트 패킷 처리완료. 현재 미처리된 패킷 사이즈 : %d\n", packetSize, m_stPacket.size);
	}
}

int Session::ReceivePacket()
{
	int freeSpace = sizeof(m_packet.buffer) - m_packet.endPos;
	int recvSize = recv(m_socket, m_packet.buffer + m_packet.endPos, freeSpace, 0);
	if (recvSize == SOCKET_ERROR || recvSize == 0) return recvSize;
	int extraRecvSize = 0;
	int packetSize = 0;

	if (recvSize >= sizeof(u_short))
		packetSize = *(u_short*)(m_packet.buffer + m_packet.startPos);

	if (packetSize <= freeSpace || recvSize == 1)
	{
		m_packet.size += recvSize;
		m_packet.endPos = (m_packet.endPos + recvSize) % sizeof(m_packet.buffer);
	}
	else
	{
		extraRecvSize = recv(m_socket, m_packet.buffer, sizeof(m_packet.buffer) - m_packet.size, 0);
		if (extraRecvSize == SOCKET_ERROR || extraRecvSize == 0) return extraRecvSize;

		m_packet.size += recvSize + extraRecvSize;
		m_packet.endPos = extraRecvSize;
	}
	//printf("ReceivePacket %d : (%d ~ %d)\n", m_stPacket.size, m_stPacket.startPos, m_stPacket.endPos);

	return recvSize + extraRecvSize;
}

void Session::Packet::Init()
{
	size = 0;
	startPos = 0;
	endPos = 0;
}

void Session::Packet::MoveRight(int _byteCount)
{
	if (_byteCount <= 0 || _byteCount >= BufferSize) return;

	memcpy(tempBuffer, buffer + BufferSize - _byteCount, _byteCount);
	memcpy(tempBuffer + _byteCount, buffer, BufferSize - _byteCount);
	memcpy(buffer, tempBuffer, BufferSize);

	startPos = (startPos + _byteCount) % BufferSize;
	endPos = (endPos + _byteCount) % BufferSize;

	printf("MoveRight\n");
}
