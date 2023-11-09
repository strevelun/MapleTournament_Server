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
};

Session::Session(SOCKET _socket) :
	m_socket(_socket)
{

}

Session::~Session()
{
}

void Session::ProcessPacket(ePacketType _eType, char* _pPacket)
{
	m_mapPacketHandlerCallback[_eType](this, _pPacket);
}

void Session::SaveUnprocessedPacket(char* _pPacket, int _totalSize)
{
	memcpy(m_unpPacket.unprocessedPacket, _pPacket, _totalSize);
	m_unpPacket.size = _totalSize;
}

void Session::LoadUnprocessedPacket(char* _pPacket, int& _totalSize)
{
	if (m_unpPacket.size <= 0)
	{
		_totalSize = 0;
		return;
	}

	memcpy(_pPacket, m_unpPacket.unprocessedPacket, m_unpPacket.size);
	_totalSize = m_unpPacket.size;
	m_unpPacket.size = 0;
}
