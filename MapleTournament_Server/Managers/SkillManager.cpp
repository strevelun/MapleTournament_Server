#include "SkillManager.h"
#include <json/json.h>

#include <fstream>

SkillManager* SkillManager::m_pInst = nullptr;

SkillManager::SkillManager()
{

}

SkillManager::~SkillManager()
{

}

bool SkillManager::Init()
{
    std::ifstream file("Skill.json");
    if (!file.is_open())
    {
        printf("Skill.json 파일 열기 실패!\n");
        return false;
    }

    Json::Value jsonData;
    Json::Reader reader;

    if (!reader.parse(file, jsonData))
    {
        printf("jsonData로 파싱 실패\n");
        return false;
    }

    for (const auto& elem : jsonData)
    {
        if (!elem.isMember("name"))
        {
            printf("\"name\"키가 없습니다.\n");
            return false;
        }
        if (!elem.isMember("name"))
        {
            printf("\"coordinates\"키가 없습니다.\n");
            return false;
        }
        if (!elem.isMember("strikePower"))
        {
            printf("\"strikePower\"키가 없습니다.\n");
            return false;
        }

        if (!elem["name"].isConvertibleTo(Json::stringValue))
        {
            printf("name키의 값을 string형으로 변환 불가능\n");
            return false;
        }
        std::string name = elem["name"].asString();
        eSkillType skillType = GetSkillType(name);
        if (skillType == eSkillType::None)
        {
            printf("알 수 없는 스킬 타입\n");
            return false;
        }

        tSkill skill;
        for (const auto& c : elem["coordinates"])
        {
            if (!c[0].isConvertibleTo(Json::intValue) || !c[1].isConvertibleTo(Json::intValue))
            {
                printf("coordinates키의 값을 int형으로 변환 불가능\n");
                return false;
            }
            skill.listCoordniates.push_back({c[0].asInt(), c[1].asInt()});
        }

        if (!elem["strikePower"].isConvertibleTo(Json::intValue))
        {
            printf("strikePower키의 값을 int형으로 변환 불가능\n");
            return false;
        }
        skill.strikePower = elem["strikePower"].asInt();

        m_mapSkill[skillType] = skill;
    }
	return true;
}

const tSkill* SkillManager::GetSkillCoordinateList(eSkillType _type) const
{
    std::map<eSkillType, tSkill>::const_iterator iter = m_mapSkill.find(_type);
    if (iter == m_mapSkill.end()) return nullptr;

    return &iter->second;
}

eSkillType SkillManager::GetSkillType(const std::string& name)
{
	if (name == "Attack0") return eSkillType::Attack0;
	if (name == "Attack0_Left") return eSkillType::Attack0_Left;

    return eSkillType::None;
}
