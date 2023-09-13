#pragma once

#include "../Setting.h"
#include <winsock2.h>

#include <vector>

class Session;

struct fd_set_ex : fd_set
{
	Session* fd_array_session[FD_SETSIZE];
};

#define FD_SET_EX(fd, set, session) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set_ex FAR *)(set))->fd_array[__i] == (fd)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set_ex FAR *)(set))->fd_count) { \
        if (((fd_set_ex FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set_ex FAR *)(set))->fd_array[__i] = (fd); \
            ((fd_set_ex FAR *)(set))->fd_array_session[__i] = (session); \
            ((fd_set_ex FAR *)(set))->fd_count++; \
        } \
    } \
} while(0)

#define FD_CLR_EX(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                ((fd_set_ex FAR *)(set))->fd_array_session[__i] = \
                    ((fd_set_ex FAR *)(set))->fd_array_session[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

class Selector
{
	fd_set				m_fdReads;
	fd_set_ex			m_fdUser;
	SOCKET				m_hSocketServer;

	std::vector<Session*> m_vecClientSocket;

public:
	Selector(SOCKET _hSocketServer);
	~Selector();

	const std::vector<Session*>& Select();

	const fd_set_ex& GetFDUser() const { return m_fdUser; }

	void RemoveSocket(SOCKET _socket) { FD_CLR_EX(_socket, &m_fdUser); }
};

