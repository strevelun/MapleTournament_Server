#pragma once
#include "Packet.h"
class FailedLoginPacket :
    public Packet
{
public:
    FailedLoginPacket();
    ~FailedLoginPacket();
};

