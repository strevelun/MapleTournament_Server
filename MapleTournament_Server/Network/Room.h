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
} tMember;

class Room
{
	unsigned int m_id;
	eRoomState m_eState = eRoomState::Ready;
	wchar_t m_strTitle[20];
	std::array<tMember, 4>		 m_arrSession; // TODO : Player
	unsigned int m_sessionCount = 0;

public:
	Room(unsigned int _id, wchar_t* _strTitle);
	~Room();

	void AddSession(Session* _pSession, eMemberType _eType = eMemberType::Member);
	void LeaveSession(Session* _pSession);

	unsigned int GetId() const { return m_id; }
	eRoomState GetRoomState() const { return m_eState; }
	const wchar_t* GetRoomTitle() const { return m_strTitle; }
	unsigned int GetSessionCount() const { return m_sessionCount; }
	const tMember* GetRoomOwner() const;
	const std::array<tMember, 4>& GetMemberList() const { return m_arrSession; }
	const tMember* GetMemberInfo(Session* _pSession);

	bool IsRoomReady();
	void SendAll(char* _buffer);
};

