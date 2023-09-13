#pragma once
#include "Packet.h"
class OKLoginPacket :
    public Packet
{
public:
    OKLoginPacket(const wchar_t* _nickname);
    ~OKLoginPacket();
};

