#include "Game.h"
#include "Network/Session.h"
#include "Defines.h"
#include "Managers/SkillManager.h"
#include "Managers/SessionManager.h"
#include "Network/Session.h"
#include "Network/User.h"

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
	if (m_curTurn > GAME_MAX_TURN)
	{
		OnGameOver();
	}
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

void Game::SetSkillType(int _slot, eSkillType _type)
{
	if (_type == eSkillType::Attack0 && (_slot == 1 || _slot == 3))
		_type = eSkillType::Attack0_Left;

	if(m_arrPlayer[_slot])
		m_arrPlayer[_slot]->_eSkillType = _type;
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

int Game::UpdateNextTurn()
{
	do 
	{
		if (m_curPlayerSlot >= RoomSlotNum - 1)
		{
			m_curPlayerSlot = -1;
			break;
		}
	} while (m_arrPlayer[++m_curPlayerSlot] == nullptr);

	return m_curPlayerSlot;
}

void Game::SendAll(char* _buffer)
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i])
			send(m_arrPlayer[i]->socket, _buffer, *(ushort*)_buffer, 0);

}

eSkillType Game::Move(int _slot, eSkillType _type)
{
	tPlayer* pPlayer = FindPlayer(_slot);
	if (!pPlayer) return eSkillType::None; 

	if (_type == eSkillType::LeftMove)
	{
		if (pPlayer->xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos -= 1;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::LeftDoubleMove)
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
			_type = eSkillType::LeftMove;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::RightMove)
	{
		if (pPlayer->xpos + 1 < BoardWidth)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 1][pPlayer->slot] = pPlayer;
			pPlayer->xpos += 1;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::RightDoubleMove)
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
			_type = eSkillType::RightMove;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::UpMove)
	{
		if (pPlayer->ypos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos - 1][pPlayer->xpos][pPlayer->slot] = pPlayer;
			pPlayer->ypos -= 1;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::DownMove)
	{
		if (pPlayer->ypos + 1 < BoardHeight)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos].erase(pPlayer->slot);
			m_arrBoard[pPlayer->ypos + 1][pPlayer->xpos][pPlayer->slot] = pPlayer;
			pPlayer->ypos += 1;
		}
		else
			return eSkillType::None;
	}
	return _type;
}

void Game::GetHitPlayerList(int _slot, std::list<tPlayer*>& _list)
{
	tPlayer* pPlayer = m_arrPlayer[_slot];
	if (!pPlayer) return;

	tPlayer* pCounterPlayer = nullptr;

	const tSkill* skill = SkillManager::GetInst()->GetSkillCoordinateList(pPlayer->_eSkillType);
	
	std::list<std::pair<int, int>>::const_iterator iter = skill->listCoordniates.cbegin();
	std::list<std::pair<int, int>>::const_iterator iterEnd = skill->listCoordniates.cend();

	std::map<int, tPlayer*>::iterator boardIter;
	std::map<int, tPlayer*>::iterator boardIterEnd;

	int strikePower = skill->strikePower;

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
				if (pCounterPlayer->_eSkillType == eSkillType::Shield)
					pCounterPlayer->score += strikePower / 2;
				else
					pCounterPlayer->score += strikePower;
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

		// 방어막의 경우 현재 한바퀴 돌때까지 유지됨. 다른 스킬의 경우 쓴 후 곧바로 None
		for (int i = 0; i < RoomSlotNum; ++i)
		{
			if (m_arrPlayer[i])
				m_arrPlayer[i]->_eSkillType = eSkillType::None;
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
