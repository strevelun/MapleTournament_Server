#include "GameManager.h"
#include "../Game.h"

GameManager* GameManager::m_pInst = nullptr;

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	std::map<unsigned int, Game*>::iterator iter = m_mapGame.begin();
	std::map<unsigned int, Game*>::iterator iterEnd = m_mapGame.end();

	for (; iter != iterEnd; )
	{
		delete iter->second;
	}
}

bool GameManager::Init()
{


	return true;
}

void GameManager::AddGame(unsigned int _id, Game* _pGame)
{
	m_mapGame.insert({ _id, _pGame });
}

Game* GameManager::FindGame(unsigned int _id)
{
	std::map<unsigned int, Game*>::iterator iter = m_mapGame.find(_id);
	if (iter != m_mapGame.end())	return iter->second;

	return nullptr;
}

bool GameManager::DeleteGame(unsigned int _id)
{
	Game* pGame = FindGame(_id);
	if (!pGame)		return false;

	delete pGame;
	m_mapGame.erase(_id);

	return true;
}

void GameManager::Update()
{
	if (m_mapGame.size() == 0) return;

	std::map<unsigned int, Game*>::iterator iter = m_mapGame.begin();
	std::map<unsigned int, Game*>::iterator iterEnd = m_mapGame.end();

	for (; iter != iterEnd; )
	{
		if (iter->second->IsEnd())
		{
			delete iter->second;
			iter = m_mapGame.erase(iter);
			continue;
		}
		else
		{
			iter->second->Update();
			++iter;
		}
	}
}
