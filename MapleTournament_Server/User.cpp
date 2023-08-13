#include "User.h"

User::User(unsigned short _id)
	: m_id(_id), m_nickname(nullptr)
{
}

User::~User()
{
	if (m_nickname) delete m_nickname;
}
