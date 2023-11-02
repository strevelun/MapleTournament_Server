#pragma once

#include "Network/TCPListener.h"

class Selector;
class Session;

class ServerApp
{
private:
    TCPListener* m_pListener = nullptr;
    Selector* m_pSelector = nullptr;

public:
    ServerApp();
    ~ServerApp();

    bool Init(const char* _ip, int _port);
    void Run();

    void CloseEverything();
};

