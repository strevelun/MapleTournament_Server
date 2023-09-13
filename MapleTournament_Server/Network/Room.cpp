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

void Room::AddUser(User* _pUser)
{
	if (m_listUser.size() >= 4) return;
	m_listUser.push_back(_pUser);
}
