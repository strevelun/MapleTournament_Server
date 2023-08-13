#include "TCPNetwork.h"
#include "Selector.h"
#include "Setting.h"

TCPNetwork::TCPNetwork(const char* _serverIP, int _serverPort)
{
	WSADATA  wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)											throw L"Failed WSAStartup()";

	m_hSocketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hSocketServer == INVALID_SOCKET)													throw L"Failed socket()";

	memset(&m_servAddr, 0, sizeof(m_servAddr));
	m_servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, _serverIP, &m_servAddr.sin_addr);
	m_servAddr.sin_port = htons(_serverPort);

	if (bind(m_hSocketServer, (SOCKADDR*)&m_servAddr, sizeof(m_servAddr)) == SOCKET_ERROR)	throw L"Binding Error";

	m_pSelector = new Selector(m_hSocketServer);
}

TCPNetwork::~TCPNetwork()
{
	if (m_hSocketServer != INVALID_SOCKET)			closesocket(m_hSocketServer);
	delete  m_pSelector;
}

bool TCPNetwork::Update()
{
	try {
		std::list<SOCKET>& clientSocketList = m_pSelector->Select(); 
		while (clientSocketList.size() > 0)
		{
			ReceivePacket(clientSocketList.front());
			clientSocketList.pop_front();
		}
	}
	catch (const std::wstring& e)
	{
		std::cout << e.c_str() << '\n';
		return false;
	}
	return true;
}

void TCPNetwork::Start(int _backlog)
{
	if (listen(m_hSocketServer, _backlog) == SOCKET_ERROR) return;
}

void TCPNetwork::ReceivePacket(SOCKET _clientSocket)
{
	int					recvSize, curTotalRecvSize = 0;
	u_short				packetSize = 0;
	char				recvBuffer[255];

	recvSize = recv(_clientSocket, recvBuffer, sizeof(recvBuffer), 0);
	if (recvSize > 0)
	{
		curTotalRecvSize += recvSize;

		while (1)
		{
			packetSize = *(u_short*)recvBuffer;
			if (curTotalRecvSize < 2 || packetSize > curTotalRecvSize) break;

			//char* packet = new char[packetSize];
			//memcpy(packet, recvBuffer, packetSize);
			ProcessPacket(recvBuffer);
			//delete[] packet;

			curTotalRecvSize -= packetSize;
			memcpy(recvBuffer, recvBuffer + packetSize, curTotalRecvSize);
		}
	}
	else
	{
		m_pSelector->ClearSocketFromInfos(_clientSocket);
		closesocket(_clientSocket);
	}
}

void TCPNetwork::ProcessPacket(char* _packet)
{ // [size][type][sentbywhom][]
	char* temp = _packet;									temp += sizeof(u_short);
	u_short type = *(u_short*)temp;							temp += sizeof(u_short);

	switch ((ePacketType)type)
	{
	case ePacketType::C_Enter:
		m_packetHandler.C_Enter(temp);
		break;
	}
}
