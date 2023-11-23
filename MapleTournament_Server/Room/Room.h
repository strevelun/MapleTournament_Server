#pragma once

#include <array>
#include <WinSock2.h>

class Session;
class User;

enum class eRoomState
{
	None,
	Ready,
	InGame
};

enum class eMemberType
{
	None,
	Member,
	Owner
};

enum class eMemberState
{
	None,
	Wait,
	Ready
};

class Member
{
private:
	Session* m_pSession = nullptr;
	eMemberType		m_eType = eMemberType::None;
	eMemberState	m_eState = eMemberState::None;
	int				m_slotNumber = 0;
	int				m_characterChoice = 0;

public:
	void Init(Session* _pSession, eMemberType _eType, int _slot);

	unsigned int GetId() const;
	const wchar_t* GetNickname() const;
	eMemberType GetType() const { return m_eType; }
	eMemberState GetState() const { return m_eState; }
	SOCKET GetSocket() const;
	int GetSlot() const { return m_slotNumber; }
	int GetChoice() const { return m_characterChoice; }

	void SetType(eMemberType _eType) { m_eType = _eType; }
	void SetState(eMemberState _eState) { m_eState = _eState; }
	void SetChoice(int _characterChoice) { m_characterChoice = _characterChoice; }
};

class Room
{
public:
	static constexpr int SlotSize = 4;
	static constexpr int RoomMaxSize = 63;

private:
	unsigned int m_id = 0; 
	eRoomState m_eState = eRoomState::None;
	wchar_t m_strTitle[20];
	std::array<Member, SlotSize>		 m_arrMember;
	std::array<bool, SlotSize>		 m_arrMemberExist = {};
	unsigned int m_memberCount = 0;

public:
	Room();
	~Room();

	//void Init();

	void AddMember(Session* _pSession, eMemberType _eType = eMemberType::Member);
	void LeaveMember(Session* _pSession);

	unsigned int  GetId() const { return m_id; }
	eRoomState GetRoomState() const { return m_eState; }
	const wchar_t* GetRoomTitle() const { return m_strTitle; }
	unsigned int GetMemberCount() const { return m_memberCount; }
	const Member* GetRoomOwner() const;
	const std::array<Member, SlotSize>& GetMemberList() const { return m_arrMember; }
	const Member* GetMemberInfo(unsigned int _id);

	void SetId(unsigned int _id);
	void SetTitle(wchar_t* _strTitle);
	void SetRoomState(eRoomState _state);
	void SetMemberState(unsigned int _id, eMemberState _state);
	void SetMemberChoice(unsigned int _id, int _choice);

	bool IsMemberExist(int _slot);
	bool IsRoomReady();
	void Send(char* _buffer, int _slot);
	void SendAll(char* _buffer, Session* _pExceptSession = nullptr);
};

