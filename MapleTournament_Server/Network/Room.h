#pragma once

#include <array>

class Session;

enum class eRoomState
{
	None,
	Ready,
	InGame
};

enum class eMemberType
{
	None,
	Member,
	Owner
};

enum class eMemberState
{
	None,
	Wait,
	Ready
};

typedef struct _tMember
{
	Session* pSession = nullptr; 
	eMemberType _eType = eMemberType::None;
	eMemberState _eState = eMemberState::None;
	int slotNumber = 0;
	int characterChoice = 0;
} tMember;

class Room
{
	unsigned int m_id;
	eRoomState m_eState = eRoomState::Ready;
	wchar_t m_strTitle[20];
	std::array<tMember, 4>		 m_arrMember; // TODO : Player
	unsigned int m_memberCount = 0;

public:
	Room(unsigned int _id, wchar_t* _strTitle);
	~Room();

	void AddSession(Session* _pSession, eMemberType _eType = eMemberType::Member);
	void LeaveSession(Session* _pSession);

	unsigned int  GetId() const { return m_id; }
	eRoomState GetRoomState() const { return m_eState; }
	const wchar_t* GetRoomTitle() const { return m_strTitle; }
	unsigned int GetMemberCount() const { return m_memberCount; }
	const tMember* GetRoomOwner() const;
	const std::array<tMember, 4>& GetMemberList() const { return m_arrMember; }
	const tMember* GetMemberInfo(Session* _pSession);

	void SetRoomState(eRoomState _state);
	void SetMemberState(Session* _pSession, eMemberState _state);

	void SetMemberChoice(Session* _pSession, int _choice);

	bool IsRoomReady();
	void SendAll(char* _buffer);
};

