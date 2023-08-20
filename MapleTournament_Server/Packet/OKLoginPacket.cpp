#include "OKLoginPacket.h"

OKLoginPacket::OKLoginPacket()
{
	ushort count = sizeof(ushort);
	*(ushort*)(m_packetBuffer + count) = (ushort)ePacketType::S_OKLogin;		count += sizeof(ushort);
	*(ushort*)m_packetBuffer = count;
}

OKLoginPacket::~OKLoginPacket()
{
}
