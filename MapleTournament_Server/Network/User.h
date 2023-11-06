#pragma once

#include <string>

class User
{
private:
	wchar_t		m_nickname[20];
	unsigned int		m_hitCount = 0;

public:
	User();
	~User();

	const wchar_t* GetNickname() const { return m_nickname; }
	void SetNickname(wchar_t* _strNickname) { wcsncpy_s(m_nickname, sizeof(m_nickname) / sizeof(wchar_t), _strNickname, sizeof(m_nickname) / sizeof(wchar_t) - 1); }

	void AddHitCount(unsigned int _hitCount) { m_hitCount += _hitCount; }

	unsigned int GetHitCount() const { return m_hitCount; }
};

