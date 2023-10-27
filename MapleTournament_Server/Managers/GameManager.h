#pragma once

#include <map>

#include "../Defines.h"

class Game;

class GameManager
{
private:
	std::map<unsigned int, Game*> m_mapGame;

public:
	bool Init();

	void AddGame(unsigned int _id, Game* _pGame);
	Game* FindGame(unsigned int _id);
	bool DeleteGame(unsigned int _id);

	void Update();

	SINGLETON(GameManager)
};

