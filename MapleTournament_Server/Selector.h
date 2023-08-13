#pragma once

#include "Setting.h"

#include <list>

class Selector
{
	fd_set				m_fdSocketInfos, m_fdReads;
	SOCKET				m_hSocketServer;

	std::list<SOCKET> m_clientSocketList;

public:
	Selector(SOCKET _hSocketServer);
	~Selector();

	std::list<SOCKET>& Select();
	fd_set& GetSocketInfos() { return m_fdSocketInfos; }

	std::list<SOCKET>& GetClientSocketList() { return m_clientSocketList; } // TODO

	void ClearSocketFromInfos(SOCKET _socket) { FD_CLR(_socket, &m_fdSocketInfos); }
};

