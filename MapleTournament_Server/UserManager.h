#pragma once

#include <map>

class User;

class UserManager
{
private:
	static UserManager* m_inst;

	std::map<uint16_t, User*> m_mapUser; // key : SOCKET ¹øÈ£

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

	bool AddUser(unsigned short _id, User* _pUser);
	User* GetUser(unsigned short _id);
};

