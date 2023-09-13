#include "OKLoginPacket.h"

OKLoginPacket::OKLoginPacket(const wchar_t* _nickname)
{
	ushort count = sizeof(ushort);
	*(ushort*)(m_packetBuffer + count) = (ushort)ePacketType::S_OKLogin;		count += sizeof(ushort);
	memcpy(m_packetBuffer + count, _nickname, wcslen(_nickname) * 2);						count += (ushort)wcslen(_nickname) * 2;
	*(wchar_t*)(m_packetBuffer + count) = L'\0';											count += 2;
	*(ushort*)m_packetBuffer = count;
}

OKLoginPacket::~OKLoginPacket()
{
}
