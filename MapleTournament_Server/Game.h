#pragma once

#include <WinSock2.h>
#include <array>
#include <vector>
#include <map>
#include <list>

#include "Setting.h"

class Session;

#define HPMax				20
#define MPMax				10

typedef struct _tPlayer
{
	SOCKET socket = 0;
	bool ready = false;
	bool standby = false;
	int score = 0; // 때린 횟수
	int slot = 0;
	int hp = 20;
	int mana = 10;
	int xpos = 0, ypos = 0;
	bool alive = true;
	bool waitForPortal = false;
	eSkillName _eSkillName = eSkillName::None;
} tPlayer;

class Game
{
public:
	static constexpr int RoomSlotNum = 4;
	static constexpr int BoardWidth = 5;
	static constexpr int BoardHeight = 4;

private:
	std::array<std::array<std::map<int, tPlayer*>, BoardWidth>, BoardHeight> m_arrBoard;
	std::array<tPlayer*, RoomSlotNum> m_arrPlayer;

	std::pair<int, int> m_portalPosition;

	bool m_isEnd = false;
	int m_curPlayerSlot = -1;
	unsigned int m_curTurn = 1;
	bool m_isItTimeForPortalCreation = false;

public:
	Game();
	~Game();

	void Update();

	void AddPlayer(tPlayer* _pPlayer);
	tPlayer* FindPlayer(int _slot);
	tPlayer* FindPlayer(Session* _pSession);
	bool RemovePlayer(int _slot);

	int CountAlivePlayer();

	unsigned int GetCurTurn() const { return m_curTurn; }
	int	GetCurPlayerSlot() const { return m_curPlayerSlot; }
	eSkillName GetCurSkillName(int _slot) const;

	void SetSkillName(int _slot, eSkillName _eName);
	void SetPortalPosition(int _xpos, int _ypos);

	void IncreaseCurTurn() { m_curTurn++; }
	
	bool IsAllReady() const;
	bool IsAllStandby() const;
	bool IsEnd() const { return m_isEnd; }

	void CheckPortal(int _slot);

	int UpdateNextTurn();
	void UpdatePortal();

	void SendAll(char* _buffer);

public:
	eMoveName Move(int _slot, eMoveName _name);// slot으로 플레이어 구분
	void GetHitPlayerList(int _slot, std::list<tPlayer*>& _list, std::list<tPlayer*>& _listDead);
	void OnNextTurn();

private:
	void OnGameOver();
};

