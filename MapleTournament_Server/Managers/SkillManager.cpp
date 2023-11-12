#include "SkillManager.h"
#include "../Skill.h"
#include "../SkillAttack.h"
#include "../SkillHeal.h"

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

    Skill* pSkill = nullptr;
    int i = 0;

    for (const auto& elem : jsonData)
    {
        if (!CheckKeyExistsCommon(elem, i)) return false;
        if (!CheckKeyConvertibleCommon(elem, i)) return false;

        eSkillType type = GetSkillType(elem["type"].asString());
        if (type == eSkillType::None)
        {
            printf("%d : 알 수 없는 스킬 타입\n", i);
            return false;
        }
      
        eSkillName name = GetSkillName(elem["name"].asString());
        if (name == eSkillName::None)
        {
            printf("%d : 알 수 없는 스킬 이름\n", i);
            return false;
        }
       
        int mana = elem["mana"].asInt();

        if (type == eSkillType::Attack)
        {
            if (!CheckKeyExistsAttack(elem, i)) return false;
            if (!CheckKeyConvertibleAttack(elem, i)) return false;

            pSkill = new SkillAttack(type, mana);

            for (const auto& c : elem["coordinates"])
                ((SkillAttack*)pSkill)->m_listCoordinates.push_back({ c[0].asInt(), c[1].asInt() });

            ((SkillAttack*)pSkill)->m_strikePower = elem["strikePower"].asInt();
            ((SkillAttack*)pSkill)->m_inversed = elem["inversed"].asBool();
        }
        else if (type == eSkillType::Heal)
        {
            if (!CheckKeyExistsHeal(elem, i)) return false;
            if (!CheckKeyConvertibleHeal(elem, i)) return false;

            pSkill = new SkillHeal(type, mana);

            ((SkillHeal*)pSkill)->m_heal = elem["heal"].asInt();
        }

        m_mapSkill[name] = pSkill;
        ++i;
    }
	return true;
}

const Skill* SkillManager::GetSkill(int _slot, eSkillName _type) const
{
    std::map<eSkillName, Skill*>::const_iterator iter = m_mapSkill.find(_type);
    if (iter == m_mapSkill.end()) return nullptr;

    if (_slot == 1 || _slot == 3)
    {
        SkillAttack* pAttack = dynamic_cast<SkillAttack*>(iter->second);
        if (pAttack && pAttack->IsInversed())
            ++iter;
    }

    return iter->second;
}

void SkillManager::GetSkillsNotAvailable(int _playerMP, std::list<eSkillName>& _listSkillName)
{
    std::map<eSkillName, Skill*>::const_iterator iter = m_mapSkill.cbegin();
    std::map<eSkillName, Skill*>::const_iterator iterEnd = m_mapSkill.cend();

    for (; iter != iterEnd; ++iter)
    {
        if (_playerMP < iter->second->GetMana())
            _listSkillName.push_back(iter->first);
    }
}

eSkillName SkillManager::GetSkillName(const std::string& _name)
{
	if (_name == "Attack0") return eSkillName::Attack0;
	if (_name == "Attack0_Left") return eSkillName::Attack0_Left;
	if (_name == "Attack1") return eSkillName::Attack1;
	if (_name == "Attack2") return eSkillName::Attack2;
	if (_name == "Attack3") return eSkillName::Attack3;
	if (_name == "Heal0") return eSkillName::Heal0;

    return eSkillName::None;
}

eSkillType SkillManager::GetSkillType(const std::string& _name)
{
    if (_name == "Attack") return eSkillType::Attack;
    if (_name == "Heal") return eSkillType::Heal;

    return eSkillType::None;
}

bool SkillManager::CheckKeyExistsCommon(const Json::Value& _elem, int _i)
{
    if (!_elem.isMember("type"))
    {
        printf("%d : \"type\"키가 없습니다.\n", _i);
        return false;
    }
    if (!_elem.isMember("name"))
    {
        printf("%d : \"name\"키가 없습니다.\n", _i);
        return false;
    }
    if (!_elem.isMember("mana"))
    {
        printf("%d : \"mana\"키가 없습니다.\n", _i);
        return false;
    }
    return true;
}

bool SkillManager::CheckKeyConvertibleCommon(const Json::Value& _elem, int _i)
{
    if (!_elem["type"].isConvertibleTo(Json::stringValue))
    {
        printf("%d : type키의 값을 string형으로 변환 불가능\n", _i);
        return false;
    }
    if (!_elem["name"].isConvertibleTo(Json::stringValue))
    {
        printf("%d : name키의 값을 string형으로 변환 불가능\n", _i);
        return false;
    }
    if (!_elem["mana"].isConvertibleTo(Json::intValue))
    {
        printf("%d : mana키의 값을 int형으로 변환 불가능\n", _i);
        return false;
    }
    return true;
}

bool SkillManager::CheckKeyExistsAttack(const Json::Value& _elem, int _i)
{
    if (!_elem.isMember("coordinates"))
    {
        printf("%d : \"coordinates\"키가 없습니다.\n", _i);
        return false;
    }
    if (!_elem.isMember("strikePower"))
    {
        printf("%d : \"strikePower\"키가 없습니다.\n", _i);
        return false;
    }
    if (!_elem.isMember("inversed"))
    {
        printf("%d : \"inversed\"키가 없습니다.\n", _i);
        return false;
    }
    return true;
}

bool SkillManager::CheckKeyConvertibleAttack(const Json::Value& _elem, int _i)
{
    for (const auto& c : _elem["coordinates"])
    {
        if (!c[0].isConvertibleTo(Json::intValue) || !c[1].isConvertibleTo(Json::intValue))
        {
            printf("%d : coordinates키의 값을 int형으로 변환 불가능\n", _i);
            return false;
        }
    }
    if (!_elem["strikePower"].isConvertibleTo(Json::intValue))
    {
        printf("%d : strikePower키의 값을 int형으로 변환 불가능\n", _i);
        return false;
    }
    if (!_elem["inversed"].isConvertibleTo(Json::booleanValue))
    {
        printf("%d : inversed키의 값을 bool형으로 변환 불가능\n", _i);
        return false;
    }

    return true;
}

bool SkillManager::CheckKeyExistsHeal(const Json::Value& _elem, int _i)
{
    if (!_elem.isMember("heal"))
    {
        printf("%d : \"heal\"키가 없습니다.\n", _i);
        return false;
    }
    return true;
}

bool SkillManager::CheckKeyConvertibleHeal(const Json::Value& _elem, int _i)
{
    if (!_elem["heal"].isConvertibleTo(Json::intValue))
    {
        printf("%d : heal키의 값을 int형으로 변환 불가능\n", _i);
        return false;
    }
    return true;
}
