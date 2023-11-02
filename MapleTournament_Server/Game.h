#pragma once

#include <WinSock2.h>
#include <array>
#include <vector>

#include "Setting.h"

typedef struct _tPlayer
{
	SOCKET socket = 0;
	bool ready = false;
	int hp = 10;
	int slot = 0;
	int xpos = 0, ypos = 0;
} tPlayer;

enum class eGameState
{
	None,
	Choice,
	UseSkill
};

class Game
{
public:
	static constexpr int RoomSlotNum = 4;

private:
	std::array<std::array<tPlayer*, 5>, 4> m_arrBoard;
	std::array<tPlayer*, RoomSlotNum> m_arrPlayer;

	bool m_isEnd = false;
	int m_curPlayerSlot = -1;
	unsigned int m_curTurn = 1;

	eGameState m_eGameState = eGameState::None;

public:
	Game();
	~Game();

	void Update();

	void AddPlayer(tPlayer* _pPlayer);
	tPlayer* FindPlayer(int _slot);
	bool RemovePlayer(int _slot);

	void SetGameState(eGameState _state) { m_eGameState = _state; }

	eGameState GetGameState() const { return m_eGameState; }
	unsigned int GetCurTurn() const { return m_curTurn; }
	int	GetCurPlayerSlot() const { return m_curPlayerSlot; }

	void IncreaseCurTurn() { m_curTurn++; }
	
	bool IsAllReady() const;
	bool IsEnd() const { return m_isEnd; }

	int UpdateNextTurn();

	void SendAll(char* _buffer);

public:
	eSkillType Move(int _slot, eSkillType _type);// slot���� �÷��̾� ����
	void OnNextTurn();
};

