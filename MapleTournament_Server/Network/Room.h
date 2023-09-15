#pragma once

#include <array>

class User;

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
	User* pUser = nullptr; // TODO : Change to SOCKET
	eMemberType _eType = eMemberType::None;
	eMemberState _eState = eMemberState::None;
} tMember;

class Room
{
	unsigned int m_id;
	eRoomState m_eState = eRoomState::Ready;
	wchar_t m_strTitle[20];
	std::array<_tMember, 4>		 m_arrUser;
	unsigned int m_userCount = 0;

public:
	Room(unsigned int _id, wchar_t* _strTitle);
	~Room();

	void AddUser(User* _pUser, eMemberType _eType = eMemberType::Member);
	void LeaveUser(User* _pUser);

	unsigned int GetId() const { return m_id; }
	eRoomState GetRoomState() const { return m_eState; }
	const wchar_t* GetRoomTitle() const { return m_strTitle; }
	unsigned int GetUserCount() const { return m_userCount; }
	User* GetRoomOwner() const;
	const std::array<_tMember, 4>& GetUserList() const { return m_arrUser; }
};

