#pragma once

#include "../Setting.h"

typedef unsigned short ushort;

class Packet
{
protected:
	char m_packetBuffer[255];

public:
	Packet();
	~Packet();

	ushort GetPacketSize() const { return *(ushort*)m_packetBuffer; }
	const char* GetPacketBuffer() const { return m_packetBuffer; }
};

