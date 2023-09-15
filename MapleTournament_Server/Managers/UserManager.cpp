#include "UserManager.h"
#include "../Network/User.h"

UserManager* UserManager::m_inst = nullptr;

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

User* UserManager::CreateUser(wchar_t* _nickname)
{
	if (FindUser(_nickname)) return nullptr;

	User* pUser = new User;
	pUser->SetNickname(_nickname);

	m_mapUser.insert({ _nickname, pUser });
	return pUser;
}

User* UserManager::FindUser(const wchar_t* _nickname)
{
	std::map<std::wstring, User*>::iterator iter = m_mapUser.find(_nickname);
	if (iter != m_mapUser.end())
		return iter->second;
	return nullptr;
}
