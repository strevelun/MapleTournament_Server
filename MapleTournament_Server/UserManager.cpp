#include "UserManager.h"

#include <cstdio>

UserManager* UserManager::m_inst = nullptr;

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

bool UserManager::AddUser(unsigned short _id, User* _pUser)
{
	if (m_mapUser.find(_id) != m_mapUser.end()) {
		printf("이미 id=%d인 유저가 존재함\n", _id);
		return false;
	}

	m_mapUser.insert({ _id, _pUser });
	return true;
}

User* UserManager::GetUser(unsigned short _id)
{
	std::map<unsigned short, User*>::const_iterator pUser = m_mapUser.find(_id);
	if (pUser == m_mapUser.end()) return nullptr;

	return pUser->second;
}
