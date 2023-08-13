#pragma once
class User
{
private:
	unsigned short m_id;
	wchar_t* m_nickname;

public:
	User(unsigned short _id);
	~User();

	void SetNickname(wchar_t* _nickname) { if (m_nickname) delete m_nickname; m_nickname = _nickname; }
};

