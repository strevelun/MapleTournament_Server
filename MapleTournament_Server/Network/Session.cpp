#include "Session.h"
#include "../Packet/PacketHandler.h"

#include <WinSock2.h>

std::map<ePacketType, void(*)(Session*, char*)> Session::m_mapPacketHandlerCallback = {
	{ ePacketType::C_EnterLobby, PacketHandler::C_EnterLobby },
	{ ePacketType::C_OKLogin, PacketHandler::C_OKLogin },
	{ ePacketType::C_Exit, PacketHandler::C_Exit },
	{ ePacketType::C_CreateRoom, PacketHandler::C_CreateRoom },
	{ ePacketType::C_Chat, PacketHandler::C_Chat },
	{ ePacketType::C_JoinRoom, PacketHandler::C_JoinRoom }
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
	if (m_unpPacket.size <= 0) return;
	memcpy(_pPacket, m_unpPacket.unprocessedPacket, m_unpPacket.size);
	_totalSize = m_unpPacket.size;
	m_unpPacket.size = 0;
}
