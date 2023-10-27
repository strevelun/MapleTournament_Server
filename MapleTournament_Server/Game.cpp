#include "Game.h"

Game::Game()
{
}

Game::~Game()
{
}

void Game::AddPlayer(tPlayer* _pPlayer)
{
	m_listPlayer.push_back(_pPlayer);
}

tPlayer* Game::FindPlayer(int _slot)
{
	std::list<tPlayer*>::iterator iter = m_listPlayer.begin();
	std::list<tPlayer*>::iterator iterEnd = m_listPlayer.end();

	for (; iter != iterEnd; iter++)
	{
		if ((*iter)->slot == _slot)
		{
			return *iter;
		}
	}
	return nullptr;
}

bool Game::RemovePlayer(int _slot)
{
	std::list<tPlayer*>::iterator iter = m_listPlayer.begin();
	std::list<tPlayer*>::iterator iterEnd = m_listPlayer.end();

	for (; iter != iterEnd; iter++)
	{
		if ((*iter)->slot == _slot)
		{
			m_arrBoard[(*iter)->ypos][(*iter)->xpos] = nullptr;
			m_listPlayer.erase(iter);
			delete* iter;
			return true;
		}
	}
	return false;
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
