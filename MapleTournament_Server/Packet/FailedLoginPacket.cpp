#include "FailedLoginPacket.h"

FailedLoginPacket::FailedLoginPacket()
{
	ushort count = sizeof(ushort);
	*(ushort*)(m_packetBuffer + count) = (ushort)ePacketType::S_FailedLogin;		count += sizeof(ushort);
	*(ushort*)m_packetBuffer = count;
}

FailedLoginPacket::~FailedLoginPacket()
{
}
