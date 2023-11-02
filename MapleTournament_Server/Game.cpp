#include "Game.h"
#include "Defines.h"

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
		m_isEnd = true;

		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_GameOver;			count += sizeof(ushort);
		*(ushort*)buffer = count;
		SendAll(buffer);
	}
}

void Game::AddPlayer(tPlayer* _pPlayer)
{
	m_arrPlayer[_pPlayer->slot] = _pPlayer;
}

tPlayer* Game::FindPlayer(int _slot)
{
	return m_arrPlayer[_slot];
}

bool Game::RemovePlayer(int _slot)
{
	delete m_arrPlayer[_slot];
	m_arrPlayer[_slot] = nullptr;
	return true;
}

bool Game::IsAllReady() const
{
	for (int i = 0; i < RoomSlotNum; i++)
		if (m_arrPlayer[i] && m_arrPlayer[i]->ready == false)
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
	if (!pPlayer) return eSkillType::None ; 

	if (_type == eSkillType::LeftMove)
	{
		if (pPlayer->xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 1] = pPlayer;
			pPlayer->xpos -= 1;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::LeftDoubleMove)
	{
		if (pPlayer->xpos - 2 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 2] = pPlayer;
			pPlayer->xpos -= 2;
		}
		else if (pPlayer->xpos - 1 >= 0)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos - 1] = pPlayer;
			pPlayer->xpos -= 1;
			_type = eSkillType::LeftMove;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::RightMove)
	{
		if (pPlayer->xpos + 1 < 5)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 1] = pPlayer;
			pPlayer->xpos += 1;
		}
		else
			return eSkillType::None;
	}
	else if (_type == eSkillType::RightDoubleMove)
	{
		if (pPlayer->xpos + 2 < 5)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 2] = pPlayer;
			pPlayer->xpos += 2;
		}
		else if (pPlayer->xpos + 1 < 5)
		{
			m_arrBoard[pPlayer->ypos][pPlayer->xpos] = nullptr;
			m_arrBoard[pPlayer->ypos][pPlayer->xpos + 1] = pPlayer;
			pPlayer->xpos += 1;
			_type = eSkillType::RightMove;
		}
		else
			return eSkillType::None;
	}
	return _type;
}
