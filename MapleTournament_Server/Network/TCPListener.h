#pragma once

#include "../Setting.h"
#include <WinSock2.h>

class TCPListener
{
private:
	SOCKET			m_socket;
	SOCKADDR_IN		m_servAddr;

public:
	TCPListener(const char* _ip, unsigned short _port);
	~TCPListener();

	bool Start(int _backlog = 5);
	SOCKET Accept();

	SOCKET GetSocket() { return m_socket; }
};