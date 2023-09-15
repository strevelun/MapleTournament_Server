#include "Room.h"
#include "User.h"

Room::Room(unsigned int _id, wchar_t* _strTitle)
	: m_id(_id)
{
	wcsncpy_s(m_strTitle, sizeof(m_strTitle) / sizeof(wchar_t), _strTitle, sizeof(m_strTitle) / sizeof(wchar_t) - 1);
}

Room::~Room()
{
	
}

void Room::AddUser(User* _pUser, eMemberType _eType)
{
	if (GetUserCount() >= 4) return;

	unsigned int size = m_arrUser.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrUser[i].pUser == nullptr)
		{
			m_arrUser[i].pUser = _pUser;
			m_arrUser[i]._eType = _eType;
			m_arrUser[i]._eState = _eType == eMemberType::Owner ? eMemberState::Ready : eMemberState::Wait;
			m_userCount++;
			return;
		}
	}
}

// 자신이 방장인지 체크
// 방장이면 다른사람이 방장
void Room::LeaveUser(User* _pUser)
{
	unsigned int size = m_arrUser.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrUser[i].pUser == _pUser)
		{
			m_arrUser[i].pUser = nullptr;
			m_arrUser[i]._eType = eMemberType::None;
			m_arrUser[i]._eState = eMemberState::None;
			m_userCount--;
			return;
		}
	}
}
/*
unsigned int Room::GetUserCount() const
{
	int count = 0;
	unsigned int size = m_arrUser.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrUser[i].pUser == nullptr) continue;
		count++;
	}
	return count;
}
*/
User* Room::GetRoomOwner() const
{
	unsigned int size = m_arrUser.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrUser[i]._eType == eMemberType::Owner)
			return m_arrUser[i].pUser;
	}
}