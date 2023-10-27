#pragma once

#include <array>
#include <list>

typedef struct _tPlayer
{
	int hp = 10;
	int slot = 0;
	int xpos = 0, ypos = 0;
} tPlayer;

class Game
{
private:
	std::array<std::array<tPlayer*, 5>, 4> m_arrBoard; // slot or tPlayer*
	std::list<tPlayer*> m_listPlayer;

public:
	Game();
	~Game();

	void AddPlayer(tPlayer* _pPlayer);
	bool RemovePlayer(int _slot);
};

