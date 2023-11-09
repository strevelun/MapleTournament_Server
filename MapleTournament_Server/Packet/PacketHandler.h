#pragma once

class Session;

class PacketHandler
{
public:
	static void C_OKLogin(Session* _pSession, char* _packet);
	static void C_Exit(Session* _pSession, char* _packet);
	static void C_CreateRoom(Session* _pSession, char* _packet);
	static void C_Chat(Session* _pSession, char* _packet);
	static void C_JoinRoom(Session* _pSession, char* _packet);
	static void C_LeaveRoom(Session* _pSession, char* _packet);
	static void C_CheckRoomReady(Session* _pSession, char* _packet);
	static void C_UserRoomReady(Session* _pSession, char* _packet);
	static void C_InGameReady(Session* _pSession, char* _packet);
	static void C_UpdateUserListPage(Session* _pSession, char* _packet);
	static void C_UpdateRoomListPage(Session* _pSession, char* _packet);
	static void C_UpdateUserSlot(Session* _pSession, char* _packet);
	static void C_Skill(Session* _pSession, char* _packet);
	static void C_NextTurn(Session* _pSession, char* _packet);
	static void C_GameOver(Session* _pSession, char* _packet);
	static void C_LobbyInit(Session* _pSession, char* _packet);
	static void C_Standby(Session* _pSession, char* _packet);
	static void C_CheckHit(Session* _pSession, char* _packet);
	static void C_CheckHeal(Session* _pSession, char* _packet);
};

