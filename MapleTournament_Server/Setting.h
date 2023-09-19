#ifndef __SETTING_H__
#define __SETTING_H__

enum class ePacketType
{
	None,
	C_EnterLobby,
	S_EnterLobby,
	C_Exit,
	S_Exit,
	C_OKLogin,
	S_OKLogin,
	S_FailedLogin,
	C_CreateRoom,
	S_CreateRoom,
	S_NotifyCreateRoom,
	S_SendSessions,
	S_SendRooms,
	S_EnterOtherUser,
	C_Chat,
	S_Chat,
	C_JoinRoom,
	S_JoinRoom,
	S_JoinRoomFail,
	C_LeaveRoom,
	S_LeaveRoom,
	S_NotifyJoinedUser,
	C_CheckRoomReady,
	S_CheckRoomReadyOK,
	S_CheckRoomReadyFail,
};

enum class eSessionState
{
	None,
	Login,
	Lobby,
	WatingRoom,
	InGame
};

#endif