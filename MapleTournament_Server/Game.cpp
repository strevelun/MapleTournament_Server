#include "Game.h"
#include "Network/Session.h"
#include "Defines.h"
#include "Managers/SkillManager.h"
#include "Managers/SessionManager.h"
#include "Network/Session.h"
#include "Network/User.h"
#include "Skill.h"
#include "SkillAttack.h"

typedef unsigned short ushort;

Game::Game()
{
	srand((unsigned int)time(0));

	for (int i = 0; i < RoomSlotNum; i++)
		m_arrPlayer[i] = nullptr;
}

Game::~Game()
{
	for (int i = 0; i < RoomSlotNum; i++)
		if(m_arrPlayer[i])
			delete m_arrPlayer[i];
}

void Game::Update()
{
	int count = CountAlivePlayer();
	if (m_curTurn > GAME_MAX_ROUND || count  <= 1)
	{
		OnGameOver();
	}

	UpdatePortal();
}

void Game::AddPlayer(tPlayer* _pPlayer)
{
	m_arrPlayer[_pPlayer->slot] = _pPlayer;
	m_arrBoard[_pPlayer->ypos][_pPlayer->xpos][_pPlayer->slot] = _pPlayer;
}

tPlayer* Game::FindPlayer(int _slot)
{
	return m_arrPlayer[_slot];
}

tPlayer* Game::FindPlayer(Session* _pSession)
{
	for (int i = 0; i < RoomSlotNum; i++)
	{
		if (m_arrPlayer[i] && m_arrPlayer[i]->socket == _pSession->GetSocket())
			return m_arrPlayer[i];
	}
	return nullptr;
}

bool Game::RemovePlayer(int _slot)
{
	delete m_arrPlayer[_slot];
	m_arrPlayer[_slot] = nullptr;
	return true;
}

int Game::CountAlivePlayer()
{
	int count = 0;
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i] && m_arrPlayer[i]->alive == true)
			++count;
	return count;
}

eSkillName Game::GetCurSkillType(int _slot) const
{
	return m_arrPlayer[_slot]->_eSkillName;
}

void Game::SetSkillType(int _slot, eSkillName _eName)
{
	if(m_arrPlayer[_slot])
		m_arrPlayer[_slot]->_eSkillName = _eName;
}

void Game::SetPortalPosition(int _xpos, int _ypos)
{
	if (_xpos < -1 || _xpos >= BoardWidth) return;
	if (_ypos < -1 || _ypos >= BoardHeight) return;

	m_portalPosition.first = _xpos;
	m_portalPosition.second = _ypos;
}

bool Game::IsAllReady() const
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i] && m_arrPlayer[i]->ready == false)
			return false;
	return true;
}

bool Game::IsAllStandby() const
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i] && m_arrPlayer[i]->standby == false)
			return false;
	return true;
}

void Game::CheckPortal(int _slot)
{
	if (m_arrPlayer[_slot]->xpos == m_portalPosition.first && m_arrPlayer[_slot]->ypos == m_portalPosition.second)
	{
		// 플레이어 랜덤 이동시키고 한번 더 턴을 줌. 그리고 포탈은 비활성화
		// 서버에서 포탈 비활성화는 -1
		int newXPos;
		int newYPos;
		do {
			newXPos = rand() % BoardWidth;
			newYPos = rand() % BoardHeight;
		} while (m_arrPlayer[_slot]->xpos == newXPos && m_arrPlayer[_slot]->ypos == newYPos);
		
		m_arrPlayer[_slot]->xpos = newXPos;
		m_arrPlayer[_slot]->ypos = newYPos;

		m_arrPlayer[_slot]->waitForPortal = true;

		m_portalPosition.first = -1;
		m_portalPosition.second = -1;
	}
}

int Game::UpdateNextTurn()
{
	do 
	{
		if (m_curPlayerSlot >= RoomSlotNum - 1)
		{
			m_curPlayerSlot = -1;
			break;
		}
	} while (m_arrPlayer[++m_curPlayerSlot] == nullptr || m_arrPlayer[m_curPlayerSlot]->alive == false);

	return m_curPlayerSlot;
}

void Game::UpdatePortal()
{
	if (m_isItTimeForPortalCreation)
	{
		do {
			m_portalPosition.first = rand() % BoardWidth;
			m_portalPosition.second = rand() % BoardHeight;
		} while (m_arrBoard[m_portalPosition.second][m_portalPosition.first].size() >= 1);

		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_CreatePortal;			count += sizeof(ushort);
		*(char*)(buffer + count) = (char)m_portalPosition.first;			count += sizeof(char);
		*(char*)(buffer + count) = (char)m_portalPosition.second;			count += sizeof(char);
		*(ushort*)buffer = count;
		SendAll(buffer);

		m_isItTimeForPortalCreation = false;
	}
}

void Game::SendAll(char* _buffer)
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i])
			send(m_arrPlayer[i]->socket, _buffer, *(ushort*)_buffer, 0);

}

eMoveName Game::Move(int _slot, eMoveName _name)
{
	tPlayer* pPlayer = FindPlayer(_slot);
	if (!pPlayer) return eMoveName::None;

	if (_name == eMoveName::LeftMove)
	{
		if (pPlayer->xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos -= 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::LeftDoubleMove)
	{
		if (pPlayer->xpos - 2 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 2][pPlayer->slot] = pPlayer;
			pPlayer->xpos -= 2;
		}
		else if (pPlayer->xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos -= 1;
			_name = eMoveName::LeftMove;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::RightMove)
	{
		if (pPlayer->xpos + 1 < BoardWidth)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos += 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::RightDoubleMove)
	{
		if (pPlayer->xpos + 2 < BoardWidth)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 2][pPlayer->slot] = pPlayer;
			pPlayer->xpos += 2;
		}
		else if (pPlayer->xpos + 1 < BoardWidth)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos += 1;
			_name = eMoveName::RightMove;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::UpMove)
	{
		if (pPlayer->ypos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos - 1][pPlayer->xpos][pPlayer->slot] = pPlayer;
			pPlayer->ypos -= 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::DownMove)
	{
		if (pPlayer->ypos + 1 < BoardHeight)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos + 1][pPlayer->xpos][pPlayer->slot] = pPlayer;
			pPlayer->ypos += 1;
		}
		else
			return eMoveName::None;
	}
	return _name;
}

void Game::GetHitPlayerList(int _slot, std::list<tPlayer*>& _list)
{
	tPlayer* pPlayer = m_arrPlayer[_slot];
	if (!pPlayer) return;

	tPlayer* pCounterPlayer = nullptr;

	const Skill* pSkill = SkillManager::GetInst()->GetSkill(pPlayer->_eSkillName);
	if (pSkill->GetType() != eSkillType::Attack) return;

	const SkillAttack* pSkillAttack = static_cast<const SkillAttack*>(pSkill);

	if (_slot == 1 || _slot == 3)
		if (pSkillAttack->IsInversed())
		{
			pSkill = SkillManager::GetInst()->GetSkill(eSkillName(int(pPlayer->_eSkillName) + 1));
			pSkillAttack = static_cast<const SkillAttack*>(pSkill);
		}

	const std::list<std::pair<int, int>>& listCoordinates = pSkillAttack->GetListCoordinates();

	std::list<std::pair<int, int>>::const_iterator iter = listCoordinates.cbegin();
	std::list<std::pair<int, int>>::const_iterator iterEnd = listCoordinates.cend();

	std::map<int, tPlayer*>::iterator boardIter;
	std::map<int, tPlayer*>::iterator boardIterEnd;

	int strikePower = pSkillAttack->GetStrikePower();

	for (; iter != iterEnd; ++iter)
	{
		if (pPlayer->xpos + iter->first < 0) continue;
		if (pPlayer->xpos + iter->first >= BoardWidth) continue;
		if (pPlayer->ypos + iter->second < 0) continue;
		if (pPlayer->ypos + iter->second >= BoardHeight) continue;
		if (m_arrBoard[pPlayer->ypos + iter->second][pPlayer->xpos + iter->first].size() == 0) continue;

		boardIter = m_arrBoard[pPlayer->ypos + iter->second][pPlayer->xpos + iter->first].begin();
		boardIterEnd = m_arrBoard[pPlayer->ypos + iter->second][pPlayer->xpos + iter->first].end();

		for (; boardIter != boardIterEnd; ++boardIter)
		{
			if (boardIter->second == pPlayer) continue;

			pCounterPlayer = boardIter->second;
			if (pCounterPlayer)
			{
				pCounterPlayer->score += strikePower;
				pCounterPlayer->hp -= strikePower;
				_list.push_back(pCounterPlayer);
			}
		}
	}
}

void Game::OnNextTurn()
{
	int curPlayerSlot = UpdateNextTurn();
	if (curPlayerSlot == -1)
	{
		IncreaseCurTurn();

		if (m_curTurn > 1 && m_curTurn % 3 == 0)
		{
			m_isItTimeForPortalCreation = true;
		}

		// 방어막의 경우 현재 한바퀴 돌때까지 유지됨. 다른 스킬의 경우 쓴 후 곧바로 None
		for (int i = 0; i < RoomSlotNum; ++i)
		{
			if (m_arrPlayer[i])
				m_arrPlayer[i]->_eSkillName = eSkillName::None;
		}

		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateDashboard;			count += sizeof(ushort);
		*(char*)(buffer + count) = (char)GetCurTurn();				count += sizeof(char);
		*(ushort*)buffer = count;
		SendAll(buffer);

		curPlayerSlot = UpdateNextTurn();
	}
	tPlayer* pCurPlayer = FindPlayer(curPlayerSlot);

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateTurn;			count += sizeof(ushort);
	std::list<eSkillName> skillNameList;
	SkillManager::GetInst()->GetSkillsNotAvailable(pCurPlayer->mana, skillNameList);
	*(char*)(buffer + count) = (char)skillNameList.size();				count += sizeof(char);
	for (eSkillName name : skillNameList)
	{
		*(char*)(buffer + count) = (char)name;				count += sizeof(char);
	}
	*(ushort*)buffer = count;
	send(pCurPlayer->socket, buffer, count, 0);
}

void Game::OnGameOver()
{
	m_isEnd = true;

	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i])
		{
			Session* pSsesion = SessionManager::GetInst()->FindSession(m_arrPlayer[i]->socket);
			User* pUser = pSsesion->GetUser();
			pUser->AddHitCount(m_arrPlayer[i]->score);
		}

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_GameOver;			count += sizeof(ushort);
	*(ushort*)buffer = count;
	SendAll(buffer);
}
