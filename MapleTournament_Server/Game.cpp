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

void Game::AddPlayer(SOCKET _socket, int _slot, int _xpos, int _ypos)
{
	Player* pPlayer = new Player(_socket, _slot, _xpos, _ypos);
	m_arrPlayer[_slot] = pPlayer;
	m_arrBoard[_ypos][_xpos][_slot] = pPlayer;
}

Player* Game::FindPlayer(int _slot)
{
	return m_arrPlayer[_slot];
}

Player* Game::FindPlayer(Session* _pSession)
{
	for (int i = 0; i < RoomSlotNum; i++)
	{
		if (m_arrPlayer[i] && m_arrPlayer[i]->m_socket == _pSession->GetSocket())
			return m_arrPlayer[i];
	}
	return nullptr;
}

bool Game::RemovePlayer(int _slot)
{
	RemovePlayerFromBoard(_slot);
	delete m_arrPlayer[_slot];
	m_arrPlayer[_slot] = nullptr;
	return true;
}

void Game::RemovePlayerFromBoard(int _slot)
{
	m_arrBoard[m_arrPlayer[_slot]->m_ypos][m_arrPlayer[_slot]->m_xpos].erase(m_arrPlayer[_slot]->m_slot);
}

int Game::CountAlivePlayer()
{
	int count = 0;
	for (int i = 0; i < RoomSlotNum; i++)
	{
		if (m_arrPlayer[i] && m_arrPlayer[i]->m_bAlive == true) ++count;
	}
	return count;
}

eSkillName Game::GetCurSkillName(int _slot) const
{
	return m_arrPlayer[_slot]->m_eSkillName;
}

void Game::SetSkillName(int _slot, eSkillName _eName)
{
	if(m_arrPlayer[_slot])
		m_arrPlayer[_slot]->m_eSkillName = _eName;
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
		if (m_arrPlayer[i] && m_arrPlayer[i]->m_bReady == false)
			return false;
	return true;
}

bool Game::IsAllStandby() const
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i] && m_arrPlayer[i]->m_bStandby == false)
			return false;
	return true;
}

void Game::CheckPortal(int _slot)
{
	if (m_arrPlayer[_slot]->m_xpos == m_portalPosition.first && m_arrPlayer[_slot]->m_ypos == m_portalPosition.second)
	{
		// 플레이어 랜덤 이동시키고 한번 더 턴을 줌. 그리고 포탈은 비활성화
		// 서버에서 포탈 비활성화는 -1
		int newXPos;
		int newYPos;
		do {
			newXPos = rand() % BoardWidth;
			newYPos = rand() % BoardHeight;
		} while (m_arrPlayer[_slot]->m_xpos == newXPos && m_arrPlayer[_slot]->m_ypos == newYPos);
		
		RemovePlayerFromBoard(_slot);

		m_arrPlayer[_slot]->m_xpos = newXPos;
		m_arrPlayer[_slot]->m_ypos = newYPos;

		m_arrBoard[newYPos][newXPos][_slot] = m_arrPlayer[_slot];

		m_arrPlayer[_slot]->m_bWaitForPortal = true;

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
	} while (m_arrPlayer[++m_curPlayerSlot] == nullptr || m_arrPlayer[m_curPlayerSlot]->m_bAlive == false);

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
			send(m_arrPlayer[i]->m_socket, _buffer, *(ushort*)_buffer, 0);
}

void Game::SendGameOverPacket()
{
	char buffer[255];
	ushort 	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_GameOver;				count += sizeof(ushort);
	char* score = (char*)(buffer + count);		count += sizeof(char);
	*(ushort*)buffer = count;

	for (int i = 0; i < RoomSlotNum; i++)
	{
		if (m_arrPlayer[i])
		{
			*score = (char)m_arrPlayer[i]->m_score;
			send(m_arrPlayer[i]->m_socket, buffer, count, 0);
		}
	}
}

eMoveName Game::Move(int _slot, eMoveName _name)
{
	Player* pPlayer = FindPlayer(_slot);
	if (!pPlayer) return eMoveName::None;

	if (_name == eMoveName::LeftMove)
	{
		if (pPlayer->m_xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos - 1][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos -= 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::LeftDoubleMove)
	{
		if (pPlayer->m_xpos - 2 >= 0)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos - 2][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos -= 2;
		}
		else if (pPlayer->m_xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos - 1][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos -= 1;
			_name = eMoveName::LeftMove;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::RightMove)
	{
		if (pPlayer->m_xpos + 1 < BoardWidth)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos + 1][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos += 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::RightDoubleMove)
	{
		if (pPlayer->m_xpos + 2 < BoardWidth)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos + 2][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos += 2;
		}
		else if (pPlayer->m_xpos + 1 < BoardWidth)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos + 1][pPlayer->m_slot] = pPlayer;
			pPlayer->m_xpos += 1;
			_name = eMoveName::RightMove;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::UpMove)
	{
		if (pPlayer->m_ypos - 1 >= 0)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos - 1][pPlayer->m_xpos][pPlayer->m_slot] = pPlayer;
			pPlayer->m_ypos -= 1;
		}
		else
			return eMoveName::None;
	}
	else if (_name == eMoveName::DownMove)
	{
		if (pPlayer->m_ypos + 1 < BoardHeight)
		{
			m_arrBoard[pPlayer->m_ypos][pPlayer->m_xpos].erase(pPlayer->m_slot);
			m_arrBoard[pPlayer->m_ypos + 1][pPlayer->m_xpos][pPlayer->m_slot] = pPlayer;
			pPlayer->m_ypos += 1;
		}
		else
			return eMoveName::None;
	}
	return _name;
}

void Game::GetHitResult(int _slot, std::list<Player*>& _list, std::list<Player*>& _listDead)
{
	Player* pPlayer = m_arrPlayer[_slot];
	if (!pPlayer) return;

	Player* pCounterPlayer = nullptr;

	const Skill* pSkill = SkillManager::GetInst()->GetSkill(_slot, pPlayer->m_eSkillName);
	if (pSkill->GetType() != eSkillType::Attack) return;

	const SkillAttack* pSkillAttack = static_cast<const SkillAttack*>(pSkill);

	const std::list<std::pair<int, int>>& listCoordinates = pSkillAttack->GetListCoordinates();

	std::list<std::pair<int, int>>::const_iterator iter = listCoordinates.cbegin();
	std::list<std::pair<int, int>>::const_iterator iterEnd = listCoordinates.cend();

	std::map<int, Player*>::iterator boardIter;
	std::map<int, Player*>::iterator boardIterEnd;

	int strikePower = pSkillAttack->GetStrikePower();

	for (; iter != iterEnd; ++iter)
	{
		if (pPlayer->m_xpos + iter->first < 0) continue;
		if (pPlayer->m_xpos + iter->first >= BoardWidth) continue;
		if (pPlayer->m_ypos + iter->second < 0) continue;
		if (pPlayer->m_ypos + iter->second >= BoardHeight) continue;
		if (m_arrBoard[pPlayer->m_ypos + iter->second][pPlayer->m_xpos + iter->first].size() == 0) continue;

		boardIter = m_arrBoard[pPlayer->m_ypos + iter->second][pPlayer->m_xpos + iter->first].begin();
		boardIterEnd = m_arrBoard[pPlayer->m_ypos + iter->second][pPlayer->m_xpos + iter->first].end();

		for (; boardIter != boardIterEnd; ++boardIter)
		{
			if (boardIter->second == pPlayer) continue;

			pCounterPlayer = boardIter->second;
			if (pCounterPlayer)
			{
				if (pCounterPlayer->m_bAlive)
				{
					pCounterPlayer->m_hp -= strikePower;
					if (pCounterPlayer->m_hp <= 0)
					{
						pCounterPlayer->m_bAlive = false;
						pCounterPlayer->m_hp = 0;
						_listDead.push_back(pCounterPlayer);
					}
					else
						_list.push_back(pCounterPlayer);
				}
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
				m_arrPlayer[i]->m_eSkillName = eSkillName::None;
		}

		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateDashboard;			count += sizeof(ushort);
		*(char*)(buffer + count) = (char)GetCurTurn();				count += sizeof(char);
		*(ushort*)buffer = count;
		SendAll(buffer);

		curPlayerSlot = UpdateNextTurn();
	}
	Player* pCurPlayer = FindPlayer(curPlayerSlot);

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateTurn;			count += sizeof(ushort);
	std::list<eSkillName> skillNameList;
	SkillManager::GetInst()->GetSkillsNotAvailable(pCurPlayer->m_mana, skillNameList);
	*(char*)(buffer + count) = (char)skillNameList.size();				count += sizeof(char);
	for (eSkillName name : skillNameList)
	{
		*(char*)(buffer + count) = (char)name;				count += sizeof(char);
	}
	*(ushort*)buffer = count;
	send(pCurPlayer->m_socket, buffer, count, 0);
}

void Game::OnGameOver()
{
	m_isEnd = true;

	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i])
		{
			Session* pSsesion = SessionManager::GetInst()->FindSession(m_arrPlayer[i]->m_socket);
			User* pUser = pSsesion->GetUser();
			pUser->AddKillCount(m_arrPlayer[i]->m_score);
		}
	SendGameOverPacket();
}

Player::Player(SOCKET _socket, int _slot, int _xpos, int _ypos) :
	m_socket(_socket), m_slot(_slot), m_xpos(_xpos), m_ypos(_ypos)
{

}
