#pragma once

#include "../Defines.h"
#include "../Setting.h"

#include <string>
#include <map>
#include <json/json.h>

class Skill;

class SkillManager
{
private:
	std::map<eSkillName, Skill*> m_mapSkill;

public:
	bool Init();
	const Skill* GetSkill(int _slot, eSkillName _type) const;
	void GetSkillsNotAvailable(int _playerMP, std::vector<eSkillName>& _listSkillName);

private:
	eSkillName GetSkillName(const std::string& _name);
	eSkillType GetSkillType(const std::string& _name);

	bool CheckKeyExistsCommon(const Json::Value& _elem, int _i);
	bool CheckKeyConvertibleCommon(const Json::Value& _elem, int _i);
	bool CheckKeyExistsAttack(const Json::Value& _elem, int _i);
	bool CheckKeyConvertibleAttack(const Json::Value& _elem, int _i);
	bool CheckKeyExistsHeal(const Json::Value& _elem, int _i);
	bool CheckKeyConvertibleHeal(const Json::Value& _elem, int _i);

	SINGLETON(SkillManager)
};

