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
	{ ePacketType::C_CreatePortal, PacketHandler::C_CreatePortal },
};

Session::Session() :
	m_socket(0)
{

}

Session::~Session()
{
}

void Session::Init(SOCKET _socket)
{
	m_buffer.Init();
	m_pUser = nullptr;
	m_pRoom = nullptr;
	m_socket = _socket;
}

void Session::ProcessPacket()
{
	int packetSize = 0, firstPartSize = 0, bufferRecvSize = 0;
	u_short type; 
	char* packetBuffer = nullptr ;

	while (1)
	{
		bufferRecvSize = m_buffer.GetRecvSize();
		if (bufferRecvSize < 2) break;
		
		packetSize = m_buffer.GetPacketSize();
		if (packetSize > bufferRecvSize) break;

		packetBuffer = m_buffer.GetPacketStartPos();					packetBuffer += sizeof(u_short);
		type = *(u_short*)packetBuffer;									packetBuffer += sizeof(u_short);
		m_mapPacketHandlerCallback[(ePacketType)type](this, packetBuffer);

		if ((ePacketType)type == ePacketType::C_Exit) return;

		m_buffer.CheckBufferSizeReadable();

		//printf("%d바이트 패킷 처리완료. 현재 미처리된 패킷 사이즈 : %d\n", packetSize, m_stPacket.size);
	}
}

int Session::ReceivePacket()
{
	return m_buffer.Recv(m_socket);
}

void Session::Buffer::Init()
{
	size = 0;
	startPos = 0;
	endPos = 0;
}

void Session::Buffer::CheckBufferSizeReadable()
{
	if (startPos < Buffer::BufferSize - 1) return;

	memcpy(tempBuffer, buffer + BufferSize - 1, 1);
	memcpy(tempBuffer + 1, buffer, BufferSize - 1);
	memcpy(buffer, tempBuffer, BufferSize);

	startPos = (startPos + 1) % BufferSize;
	endPos = (endPos + 1) % BufferSize;
}

int Session::Buffer::Recv(SOCKET _socket)
{
	int recvSpace = endPos < startPos ? startPos - endPos - 1 : BufferSize - endPos;
	int recvSize = recv(_socket, buffer + endPos, recvSpace, 0);
	if (recvSize == SOCKET_ERROR || recvSize == 0) return recvSize;

	size += recvSize;
	endPos = (endPos + recvSize) % BufferSize;

	return recvSize;
}

char* Session::Buffer::GetPacketStartPos()
{
	char* packetBuffer = nullptr;
	int packetSize = GetPacketSize();

	if (startPos + packetSize > BufferSize)
	{
		int firstPartSize = BufferSize - startPos;
		memcpy(tempBuffer, buffer + startPos, firstPartSize);
		memcpy(tempBuffer + firstPartSize, buffer, packetSize - firstPartSize);
		packetBuffer = tempBuffer;			
	}
	else
		packetBuffer = buffer + startPos;					

	size -= packetSize;
	startPos = (startPos + packetSize) % BufferSize;

	return packetBuffer;
}