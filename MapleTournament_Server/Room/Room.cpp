#include "Room.h"
#include "../User/User.h"
#include "../Network/Session.h"

Room::Room()
{
	
}

Room::~Room()
{
	
}

void Room::Init()
{
	for (int i = 0; i < SlotSize; i++)
	{
		m_arrMember[i].m_stInfo.slotNumber = i;
		m_arrMemberExist[i] = false;
	}
	m_memberCount = 0;
}

void Room::AddMember(Session* _pSession, eMemberType _eType)
{
	if (m_memberCount >= SlotSize) return;

	for (int i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i])
		{
			m_arrMember[i].m_id = _pSession->GetId();
			m_arrMember[i].m_stInfo.pUser = _pSession->GetUser();
			m_arrMember[i].m_socket = _pSession->GetSocket();
			m_arrMember[i].m_stInfo.eType = _eType;
			m_arrMember[i].m_stInfo.eState = _eType == eMemberType::Owner ? eMemberState::Ready : eMemberState::Wait;
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
		if (m_arrMember[i].m_id == _pSession->GetId())
		{
			if(m_arrMember[i].m_stInfo.eType == eMemberType::Owner)
				isOwner = true;
			//m_arrMember[i].m_stInfo.pUser = nullptr;
			m_arrMember[i].m_stInfo.eType = eMemberType::None;
			m_arrMember[i].m_stInfo.eState = eMemberState::None;
			m_arrMember[i].m_stInfo.characterChoice = 0;
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
			m_arrMember[i].m_stInfo.eType = eMemberType::Owner;
			break;
		}
	}
}

const Member* Room::GetRoomOwner() const
{
	for (int i = 0; i < SlotSize; i++)
	{
		if (m_arrMember[i].GetInfo().eType == eMemberType::Owner)
			return &m_arrMember[i];
	}
	return nullptr;
}

const Member* Room::GetMemberInfo(unsigned int _id)
{
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (m_arrMember[i].m_id == _id)
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
		if (m_arrMember[i].m_id == _id)
		{
			m_arrMember[i].m_stInfo.eState = _state;
			break;
		}
	}
}

void Room::SetMemberChoice(unsigned int _id, int _choice)
{
	for (size_t i = 0; i < SlotSize; i++)
	{
		if (m_arrMember[i].m_id == _id)
		{
			m_arrMember[i].m_stInfo.characterChoice = _choice;
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
		if (m_arrMember[i].m_stInfo.eState == eMemberState::Wait && m_arrMember[i].m_stInfo.eType == eMemberType::Member)
				return false;
	}
	return true;
}

void Room::SendAll(char* _buffer, Session* _pExceptSession)
{
	for (int i = 0; i < SlotSize; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (_pExceptSession && _pExceptSession->GetSocket() == m_arrMember[i].m_socket) continue;
		send(m_arrMember[i].m_socket, _buffer, *(u_short*)_buffer, 0);
	}
}