#ifndef __SETTING_H__
#define __SETTING_H__

enum class ePacketType
{
	None,
	S_EnterLobby,
	C_Exit,
	C_OKLogin,
	S_OKLogin,
	S_FailedLogin,
	C_CreateRoom,
	S_CreateRoom,
	C_Chat,
	S_Chat,
	C_JoinRoom,
	S_JoinRoom,
	S_JoinRoomFail,
	C_LeaveRoom,
	S_UpdateRoomMemberLeave,
	S_NotifyJoinedUser,
	C_CheckRoomReady,
	S_CheckRoomReadyOK,
	S_CheckRoomReadyFail,
	C_UserRoomReady,
	S_UpdateUserState,
	S_UpdateWaitingRoomBtn,
	S_UpdateUserType,
	C_InGameReady,
	S_InGameReady,
	C_UpdateUserListPage,
	S_UpdateUserListPage,
	C_UpdateRoomListPage,
	S_UpdateRoomListPage,
	C_UpdateUserSlot,
	S_UpdateUserSlot, 
	C_Skill,
	S_Skill,
	S_UpdateTurn,
	S_UpdateDashboard,
	C_NextTurn,
	S_GameOver,
	C_GameOver,
	S_GameOverSceneChange,
	C_LobbyInit,
	S_UpdateIngameUserLeave,
	S_Standby,
	C_Standby,
};

enum class eSessionState
{
	None,
	Login,
	Lobby,
	WaitingRoom,
	InGame
};

enum class eSkillType
{
	None,
	AttackCloud,
	LeftMove,
	LeftDoubleMove,
	RightMove,
	RightDoubleMove,
	UpMove,
	DownMove,

	NumOfSkills,
};

#endif