#pragma once
#include "Packet.h"
class OKLoginPacket :
    public Packet
{
public:
    OKLoginPacket();
    ~OKLoginPacket();
};

