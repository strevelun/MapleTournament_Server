#pragma once

#include "Setting.h"
#include "packet/PacketHandler.h"

/*
select된 소켓들이 보낸 패킷을 받고, 조립하고, 처리해준다.
*/

class TCPNetwork
{
private:
	SOCKADDR_IN		m_servAddr;
	SOCKET			m_hSocketServer;

	class Selector*		m_pSelector;
	PacketHandler		m_packetHandler;

public:
	TCPNetwork(const char* _serverIP, int _serverPort);
	~TCPNetwork();

	bool Update();
	void Start(int _backlog = 3);

private:
	void ReceivePacket(SOCKET _clientSocket);
	void ProcessPacket(char* _packet);
};

