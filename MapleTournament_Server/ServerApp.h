#pragma once

#include "Network/TCPListener.h"

class Selector;
class Session;

class ServerApp
{
private:
    TCPListener* m_pListener;
    Selector* m_pSelector;

public:
    ServerApp();
    ~ServerApp();

    bool Init(const char* _ip, int _port);
    void Run();

private:
    void ReceivePacket(Session* _pSession);
};

