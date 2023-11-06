#pragma once

#include "../Defines.h"
#include "../Setting.h"

#include <string>
#include <map>
#include <list>

typedef struct _tSkill
{
	std::list<std::pair<int, int>> listCoordniates;
	int strikePower = 0;
	int mp = 0;
} tSkill;

class SkillManager
{
private:
	std::map<eSkillType, tSkill> m_mapSkill;

public:
	bool Init();
	const tSkill* GetSkillCoordinateList(eSkillType _type) const;

private:
	eSkillType GetSkillType(const std::string& name);

	SINGLETON(SkillManager)
};

