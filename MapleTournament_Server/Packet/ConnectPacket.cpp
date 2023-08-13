#include "ConnectPacket.h"

ConnectPacket::ConnectPacket(ushort _clientSocket)
	//: m_id(_clientSocket)
{
	ushort count = sizeof(ushort);
	*(ushort*)(m_packetBuffer + count) = (ushort)ePacketType::S_Connect;		count += sizeof(ushort);
	*(ushort*)(m_packetBuffer + count) = _clientSocket;							count += sizeof(ushort);

	*(ushort*)m_packetBuffer = count;									
}

ConnectPacket::~ConnectPacket()
{
}
