#pragma once

#include "Setting.h"
#include "packet/PacketHandler.h"

/*
select�� ���ϵ��� ���� ��Ŷ�� �ް�, �����ϰ�, ó�����ش�.
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

