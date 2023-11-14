#include "Room.h"
#include "User.h"
#include "Session.h"

Room::Room(unsigned int _id, wchar_t* _strTitle)
	: m_id(_id)
{
	wcsncpy_s(m_strTitle, sizeof(m_strTitle) / sizeof(wchar_t), _strTitle, sizeof(m_strTitle) / sizeof(wchar_t) - 1);
	size_t size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		m_arrMember[i].m_stInfo.slotNumber = i;
		m_arrMemberExist[i] = false;
	}
}

Room::~Room()
{
	
}

void Room::AddMember(Session* _pSession, eMemberType _eType)
{
	if (m_memberCount >= 4) return;

	unsigned int size = m_arrMember.size();
	for (int i = 0; i < size; i++)
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
	unsigned int size = m_arrMember.size();
	int i = 0;
	bool isOwner = false;
	for (; i < size; i++)
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
		for (i = 0; i < size; i++)
		{
			if (!m_arrMemberExist[i]) continue;
			m_arrMember[i].m_stInfo.eType = eMemberType::Owner;
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
const Member* Room::GetRoomOwner() const
{
	unsigned int size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrMember[i].GetInfo().eType == eMemberType::Owner)
			return &m_arrMember[i];
	}
	return nullptr;
}

const Member* Room::GetMemberInfo(unsigned int _id)
{
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i].m_id == _id)
			return &m_arrMember[i];
	}
	return nullptr;
}

void Room::SetRoomState(eRoomState _state)
{
	m_eState = _state;
}

void Room::SetMemberState(unsigned int _id, eMemberState _state)
{
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
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
	size_t size = m_arrMember.size();
	for (size_t i = 0; i < size; i++)
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
	size_t size = m_arrMember.size();
	if (m_memberCount <= 1) return false;
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrMember[i].m_stInfo.eState == eMemberState::Wait && m_arrMember[i].m_stInfo.eType == eMemberType::Member)
				return false;
	}
	return true;
}

void Room::SendAll(char* _buffer, Session* _pExceptSession)
{
	size_t size = m_arrMember.size();
	for (int i = 0; i < size; i++)
	{
		if (!m_arrMemberExist[i]) continue;
		if (_pExceptSession && _pExceptSession->GetSocket() == m_arrMember[i].m_socket) continue;
		send(m_arrMember[i].m_socket, _buffer, *(u_short*)_buffer, 0);
	}
}