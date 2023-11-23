#include "Room.h"
#include "../User/User.h"
#include "../Network/Session.h"

Room::Room() :
	m_strTitle{}, m_arrMemberExist{}
{
	
}

Room::~Room()
{
	
}

void Room::AddMember(Session* _pSession, eMemberType _eType)
{
	if (m_memberCount >= SlotSize) return;

	for (int i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i])
		{
			m_arrMember[i].Init(_pSession, _eType, i);
			m_arrMemberExist[i] = true;
			m_memberCount++;
			return;
		}
	}
}

// 자신이 방장인지 체크
// 방장이면 다른사람이 방장
void Room::LeaveMember(Session* _pSession)
{
	int i = 0;
	bool isOwner = false;
	for (; i < SlotSize; i++)
	{
		if (m_arrMember[i].GetId() == _pSession->GetId())
		{
			if(m_arrMember[i].GetType() == eMemberType::Owner)
				isOwner = true;
			m_memberCount--;
			m_arrMemberExist[i] = false;
			break;
		}
	}

	if (isOwner)
	{
		for (i = 0; i < SlotSize; i++)
		{
			if (!m_arrMemberExist[i]) continue;
			m_arrMember[i].SetType(eMemberType::Owner);
			break;
		}
	}
}

const Member* Room::GetRoomOwner() const
{
	for (int i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (m_arrMember[i].GetType() == eMemberType::Owner)
			return &m_arrMember[i];
	}
	return nullptr;
}

const Member* Room::GetMemberInfo(unsigned int _id)
{
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (m_arrMember[i].GetId() == _id)
			return &m_arrMember[i];
	}
	return nullptr;
}

void Room::SetId(unsigned int _id)
{
	m_id = _id;
}

void Room::SetTitle(wchar_t* _strTitle)
{
	wcsncpy_s(m_strTitle, sizeof(m_strTitle) / sizeof(wchar_t), _strTitle, sizeof(m_strTitle) / sizeof(wchar_t) - 1);
}

void Room::SetRoomState(eRoomState _state)
{
	m_eState = _state;
}

void Room::SetMemberState(unsigned int _id, eMemberState _state)
{
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (m_arrMember[i].GetId() == _id)
		{
			m_arrMember[i].SetState(_state);
			break;
		}
	}
}

void Room::SetMemberChoice(unsigned int _id, int _choice)
{
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (m_arrMember[i].GetId() == _id)
		{
			m_arrMember[i].SetChoice(_choice);
			break;
		}
	}
}

bool Room::IsMemberExist(int _slot)
{
	return m_arrMemberExist[_slot];
}

bool Room::IsRoomReady()
{
	if (m_memberCount <= 1) return false;
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (m_arrMember[i].GetState() == eMemberState::Wait && m_arrMember[i].GetType() == eMemberType::Member)
				return false;
	}
	return true;
}

void Room::Send(char* _buffer, int _slot)
{
	if (_slot < 0 || _slot >= 4) return;
	if (!m_arrMemberExist[_slot]) return;

	send(m_arrMember[_slot].GetSocket(), _buffer, *(u_short*)_buffer, 0);
}

void Room::SendAll(char* _buffer, Session* _pExceptSession)
{
	for (int i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (_pExceptSession && _pExceptSession->GetSocket() == m_arrMember[i].GetSocket()) continue;
		send(m_arrMember[i].GetSocket(), _buffer, *(u_short*)_buffer, 0);
	}
}

void Member::Init(Session* _pSession, eMemberType _eType, int _slot)
{
	m_pSession = _pSession;
	m_eType = _eType;
	m_eState = _eType == eMemberType::Owner ? eMemberState::Ready : eMemberState::Wait;
	m_slotNumber = _slot;
	m_characterChoice = 0;
}

const wchar_t* Member::GetNickname() const
{
	return m_pSession->GetUser()->GetNickname();
}

SOCKET Member::GetSocket() const
{
	return m_pSession->GetSocket();
}

unsigned int Member::GetId() const
{
	return m_pSession->GetId();
}
