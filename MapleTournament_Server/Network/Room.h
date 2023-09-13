#pragma once

#include <list>

class User;

enum class eRoomState
{
	None,
	Ready,
	InGame
};

class Room
{
	unsigned int m_id;
	eRoomState m_eState = eRoomState::Ready;
	wchar_t m_strTitle[20];
	std::list<User*> m_listUser;	// 방장은 첫번째

public:
	Room(unsigned int _id, wchar_t* _strTitle);
	~Room();

	void AddUser(User* _pUser);

	unsigned int GetId() const { return m_id; }
	eRoomState GetRoomState() const { return m_eState; }
	const wchar_t* GetRoomTitle() const { return m_strTitle; }
	unsigned int GetUserCount() const { return m_listUser.size(); }
	User* GetFirstUser() const { return m_listUser.front(); }
};

