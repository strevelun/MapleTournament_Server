#pragma once
#include "Packet.h"
class ConnectPacket :
    public Packet
{
private:
    //ushort          m_id;

public:
    ConnectPacket(ushort _clientSocket);
    ~ConnectPacket();
};

