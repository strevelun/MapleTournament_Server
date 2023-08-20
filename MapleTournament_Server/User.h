#pragma once

#include <string>

class User
{
private:
	unsigned short m_id;
	std::wstring m_nickname;

public:
	User(unsigned short _id);
	~User();

	void SetNickname(const std::wstring& _nickname) { m_nickname = _nickname; }

	const std::wstring& GetNickname() const { return m_nickname; }
	unsigned short GetId() const { return m_id; }
};

