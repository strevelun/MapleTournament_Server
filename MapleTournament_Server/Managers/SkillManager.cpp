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
        printf("Skill.json ���� ���� ����!\n");
        return false;
    }

    Json::Value jsonData;
    Json::Reader reader;

    if (!reader.parse(file, jsonData))
    {
        printf("jsonData�� �Ľ� ����\n");
        return false;
    }

    for (const auto& elem : jsonData)
    {
        if (!elem.isMember("name"))
        {
            printf("\"name\"Ű�� �����ϴ�.\n");
            return false;
        }
        if (!elem.isMember("name"))
        {
            printf("\"coordinates\"Ű�� �����ϴ�.\n");
            return false;
        }
        if (!elem.isMember("strikePower"))
        {
            printf("\"strikePower\"Ű�� �����ϴ�.\n");
            return false;
        }

        if (!elem["name"].isConvertibleTo(Json::stringValue))
        {
            printf("nameŰ�� ���� string������ ��ȯ �Ұ���\n");
            return false;
        }
        std::string name = elem["name"].asString();
        eSkillType skillType = GetSkillType(name);
        if (skillType == eSkillType::None)
        {
            printf("�� �� ���� ��ų Ÿ��\n");
            return false;
        }

        tSkill skill;
        for (const auto& c : elem["coordinates"])
        {
            if (!c[0].isConvertibleTo(Json::intValue) || !c[1].isConvertibleTo(Json::intValue))
            {
                printf("coordinatesŰ�� ���� int������ ��ȯ �Ұ���\n");
                return false;
            }
            skill.listCoordniates.push_back({c[0].asInt(), c[1].asInt()});
        }

        if (!elem["strikePower"].isConvertibleTo(Json::intValue))
        {
            printf("strikePowerŰ�� ���� int������ ��ȯ �Ұ���\n");
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
