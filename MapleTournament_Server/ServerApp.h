#pragma once
#include "App.h"

class TCPNetwork;

class ServerApp :
    public App
{
private:
    TCPNetwork* m_pNetwork;

public:
    ServerApp(const char* _serverIP, int _serverPort);
    ~ServerApp();

    void Run() override;
};

