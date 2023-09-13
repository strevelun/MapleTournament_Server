#pragma once
#include <winsock2.h>

class Session;

class PacketHandler
{
public:
	static void C_Enter(Session* _pSession, char* _packet); // 플레이어가 닉네임 입력하면 플레이어한테서 오는 패킷을 처리하는 함수
	static void C_OKLogin(Session* _pSession, char* _packet);
	static void C_Exit(Session* _pSession, char* _packet);
	static void C_CreateRoom(Session* _pSession, char* _packet);
	static void C_Chat(Session* _pSession, char* _packet);
	static void C_JoinRoom(Session* _pSession, char* _packet);
};

