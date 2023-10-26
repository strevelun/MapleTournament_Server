#include "Room.h"
#include "User.h"
#include "Session.h"

Room::Room(unsigned int _id, wchar_t* _strTitle)
	: m_id(_id)
{
	wcsncpy_s(m_strTitle, sizeof(m_strTitle) / sizeof(wchar_t), _strTitle, sizeof(m_strTitle) / sizeof(wchar_t) - 1);
	size_t size = m_arrMember.size();
	for (int i = 0; i < size; i++)
		m_arrMember[i].slotNumber = i;
}

Room::~Room()
{
	
}

void Room::AddSession(Session* _pSession, eMemberType _eType)
{
	if (m_memberCount >= 4) return;

	unsigned int size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrMember[i].pSession == nullptr)
		{
			m_arrMember[i].pSession = _pSession;
			m_arrMember[i]._eType = _eType;
			m_arrMember[i]._eState = _eType == eMemberType::Owner ? eMemberState::Ready : eMemberState::Wait;
			m_memberCount++;
			return;
		}
	}
}

// 자신이 방장인지 체크
// 방장이면 다른사람이 방장
void Room::LeaveSession(Session* _pSession)
{
	unsigned int size = m_arrMember.size();
	int i = 0;
	bool isOwner = false;
	for (; i < size; i++)
	{
		if (m_arrMember[i].pSession == _pSession)
		{
			if(m_arrMember[i]._eType == eMemberType::Owner) 
				isOwner = true;
			m_arrMember[i].pSession = nullptr;
			m_arrMember[i]._eType = eMemberType::None;
			m_arrMember[i]._eState = eMemberState::None;
			m_arrMember[i].characterChoice = 0;
			m_memberCount--;
			break;
		}
	}

	if (isOwner)
	{
		for (i = 0; i < size; i++)
		{
			if (m_arrMember[i].pSession == nullptr) continue;
			m_arrMember[i]._eType = eMemberType::Owner;
			break;
		}
	}
}
/*
unsigned int Room::GetUserCount() const
{
	int count = 0;
	unsigned int size = m_arrUser.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrUser[i].pUser == nullptr) continue;
		count++;
	}
	return count;
}
*/
const tMember* Room::GetRoomOwner() const
{
	unsigned int size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrMember[i]._eType == eMemberType::Owner)
			return &m_arrMember[i];
	}
	return nullptr;
}

const tMember* Room::GetMemberInfo(Session* _pSession)
{
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i].pSession == _pSession)
			return &m_arrMember[i];
	}
	return nullptr;
}

void Room::SetRoomState(eRoomState _state)
{
	m_eState = _state;
}

void Room::SetMemberState(Session* _pSession, eMemberState _state)
{
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i].pSession == _pSession)
		{
			m_arrMember[i]._eState = _state;
			break;
		}
	}
}

void Room::SetMemberChoice(Session* _pSession, int _choice)
{
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i].pSession == _pSession)
		{
			m_arrMember[i].characterChoice = _choice;
			break;
		}
	}
}

bool Room::IsRoomReady()
{
	size_t size = m_arrMember.size();
	if (m_memberCount <= 1) return false;
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i]._eState == eMemberState::Wait)
				return false;
	}
	return true;
}

void Room::SendAll(char* _buffer)
{
	size_t size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrMember[i].pSession == nullptr) continue;
		send(m_arrMember[i].pSession->GetSocket(), _buffer, *(u_short*)_buffer, 0);
	}
}
