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
	S_SendSessions,
	S_SendRooms,
	S_EnterOtherUser,
	C_Chat,
	S_Chat,
	C_JoinRoom,
	S_JoinRoom,
};

#endif