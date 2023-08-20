#include "UserManager.h"
#include "User.h"

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

User* UserManager::FindUserByNickname(const std::wstring& _nickname) 
{
	std::map<uint16_t, User*>::const_iterator iter = m_mapUser.cbegin();
	std::map<uint16_t, User*>::const_iterator iterEnd = m_mapUser.cend();

	for (; iter != iterEnd; iter++) {
		if (iter->second->GetNickname() == _nickname) {
			return iter->second;
		}
	}

	return nullptr;
}
