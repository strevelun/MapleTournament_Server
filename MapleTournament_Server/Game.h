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

class Player
{
private:
	friend class Game;

private:
	SOCKET m_socket = 0;
	bool m_bReady = false;
	bool m_bStandby = false;
	int m_score = 0; // 킬 수
	int m_slot = 0;
	int m_hp = 20;
	int m_mana = 10;
	int m_xpos = 0, m_ypos = 0;
	bool m_bAlive = true;
	bool m_bWaitForPortal = false;
	eSkillName m_eSkillName = eSkillName::None;

	Player(SOCKET _socket, int _slot, int _xpos, int _ypos);

public:
	SOCKET GetSocket() const { return m_socket; }
	bool IsReady() const { return m_bReady; }
	bool IsStandby() const { return m_bStandby; }
	int GetScore() const { return m_score; }
	int GetSlot() const { return m_slot; }
	int GetHP() const { return m_hp; }
	int GetMana() const { return m_mana; }
	int GetXPos() const { return m_xpos; }
	int GetYPos() const { return m_ypos; }
	bool IsAlive() const { return m_bAlive; }
	bool IsWaitingForPortal() const { return m_bWaitForPortal; }
	eSkillName GetSkillName() const { return m_eSkillName; }

	void SetSocket(SOCKET socket) { m_socket = socket; }
	void SetReady(bool ready) { m_bReady = ready; }
	void SetStandby(bool standby) { m_bStandby = standby; }
	void SetScore(int score) { m_score = score; }
	void SetSlot(int slot) { m_slot = slot; }
	void SetHP(int hp) { m_hp = hp; }
	void SetMana(int mana) { m_mana = mana; }
	void SetXPos(int xpos) { m_xpos = xpos; }
	void SetYPos(int ypos) { m_ypos = ypos; }
	void SetAlive(bool alive) { m_bAlive = alive; }
	void SetWaitForPortal(bool waitForPortal) { m_bWaitForPortal = waitForPortal; }
	void SetSkillName(eSkillName skillName) { m_eSkillName = skillName; }
};

class Game
{
public:
	static constexpr int RoomSlotNum = 4;
	static constexpr int BoardWidth = 5;
	static constexpr int BoardHeight = 4;

private:
	std::array<std::array<std::map<int, Player*>, BoardWidth>, BoardHeight> m_arrBoard;
	std::array<Player*, RoomSlotNum> m_arrPlayer;

	std::pair<int, int> m_portalPosition;

	bool m_isEnd = false;
	int m_curPlayerSlot = -1;
	unsigned int m_curTurn = 1;
	bool m_isItTimeForPortalCreation = false;

public:
	Game();
	~Game();

	void Update();

	void AddPlayer(SOCKET _socket, int _slot, int _xpos, int _ypos);
	Player* FindPlayer(int _slot);
	Player* FindPlayer(Session* _pSession);
	bool RemovePlayer(int _slot);
	void RemovePlayerFromBoard(int _slot);

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
	void SendGameOverPacket();

	void SetPlayerMana();
	void SetPlayerReady();
	void SetPlayerWaitForPortal();
	void SetPlayerStandBy();
	void SetPlayerScore();

public:
	eMoveName Move(int _slot, eMoveName _name);// slot으로 플레이어 구분
	void GetHitResult(int _slot, std::list<Player*>& _list, std::list<Player*>& _listDead);
	void OnNextTurn();

private:
	void OnGameOver();
};

