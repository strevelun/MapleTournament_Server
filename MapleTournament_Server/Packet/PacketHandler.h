#pragma once
#include <winsock2.h>

class Session;

class PacketHandler
{
public:
	static void C_Enter(Session* _pSession, char* _packet); // �÷��̾ �г��� �Է��ϸ� �÷��̾����׼� ���� ��Ŷ�� ó���ϴ� �Լ�
	static void C_OKLogin(Session* _pSession, char* _packet);
	static void C_Exit(Session* _pSession, char* _packet);
	static void C_CreateRoom(Session* _pSession, char* _packet);
	static void C_Chat(Session* _pSession, char* _packet);
	static void C_JoinRoom(Session* _pSession, char* _packet);
};

