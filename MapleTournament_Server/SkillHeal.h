#pragma once
#include "Skill.h"
class SkillHeal :
    public Skill
{
private:
    friend class SkillManager;

private:
    int m_heal = 0;

public:
    SkillHeal(eSkillType _eType, int _mana);
    virtual ~SkillHeal();
};

