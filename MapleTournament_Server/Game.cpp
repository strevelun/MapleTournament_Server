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
