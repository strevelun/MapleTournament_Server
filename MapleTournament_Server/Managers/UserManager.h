#pragma once

#include <map>
#include <string>

class User;

class UserManager
{
private:
	static UserManager* m_inst;

	std::map<std::wstring, User*> m_mapUser;

	UserManager();
	~UserManager();

public:
	static UserManager* GetInst()
	{
		if (!m_inst)
			m_inst = new UserManager;
		return m_inst;
	}

	static void DestroyInst()
	{
		if (m_inst)
			delete m_inst;
		m_inst = nullptr;
	}

	User* CreateUser(wchar_t* _nickname);
	User* FindUser(wchar_t* _nickname);
};

