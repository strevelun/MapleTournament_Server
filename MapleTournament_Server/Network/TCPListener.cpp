#include "TCPListener.h"
#include "Selector.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

TCPListener::TCPListener(const char* _ip, unsigned short _port)
{
	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&m_servAddr, 0, sizeof(m_servAddr));
	m_servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, _ip, &m_servAddr.sin_addr);
	m_servAddr.sin_port = htons(_port);
}
 
TCPListener::~TCPListener()
{
	if (m_socket) closesocket(m_socket);
}

bool TCPListener::Start(int _backlog)
{
	if (bind(m_socket, (SOCKADDR*)&m_servAddr, sizeof(m_servAddr)) == SOCKET_ERROR)
		return false;

	if (listen(m_socket, _backlog) == SOCKET_ERROR)
		return false;
	return true;
}

SOCKET TCPListener::Accept()
{
	SOCKADDR_IN		clientAddr;
	int addrSize = sizeof(clientAddr);
	return accept(m_socket, (SOCKADDR*)&clientAddr, &addrSize);
}