#pragma once

#include <map>
#include <string>

#include "../Defines.h"

class User;

class UserManager
{
private:
	std::map<std::wstring, User*> m_mapUser;

public:
	User* CreateUser(wchar_t* _nickname);
	User* FindUser(const wchar_t* _nickname);

	SINGLETON(UserManager)
};

