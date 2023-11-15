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
	friend class Room;

private:
	struct _stInfo
	{
		User*			pUser = nullptr;
		eMemberType		eType = eMemberType::None;
		eMemberState	eState = eMemberState::None;
		int				slotNumber = 0;
		int				characterChoice = 0;
	};

private:
	unsigned int m_id = 0;
	SOCKET		m_socket;
	_stInfo		m_stInfo;

public:
	SOCKET GetSocket() const { return m_socket; }
	const _stInfo& GetInfo() const { return m_stInfo; }
};

class Room
{
public:
	static constexpr int SlotSize = 4;
private:
	unsigned int m_id = 0; 
	eRoomState m_eState = eRoomState::None;
	wchar_t m_strTitle[20];
	std::array<Member, SlotSize>		 m_arrMember;
	std::array<bool, SlotSize>		 m_arrMemberExist;
	unsigned int m_memberCount = 0;

public:
	Room();
	~Room();

	void Init();

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
	void SendAll(char* _buffer, Session* _pExceptSession = nullptr);
};

