#pragma once

#include "Setting.h"

class Skill
{
protected:
	eSkillType		m_eType = eSkillType::None;
	int				m_mana = 0;

	Skill(eSkillType _eType, int _mana);
	virtual ~Skill();

public:
	eSkillType GetType() const { return m_eType; }
};

