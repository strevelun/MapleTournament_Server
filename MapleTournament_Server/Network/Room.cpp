#include "Room.h"
#include "User.h"
#include "Session.h"

Room::Room(unsigned int _id, wchar_t* _strTitle)
	: m_id(_id)
{
	wcsncpy_s(m_strTitle, sizeof(m_strTitle) / sizeof(wchar_t), _strTitle, sizeof(m_strTitle) / sizeof(wchar_t) - 1);
	size_t size = m_arrSession.size();
	for (int i = 0; i < size; i++)
		m_arrSession[i].slotNumber = i;
}

Room::~Room()
{
	
}

void Room::AddSession(Session* _pSession, eMemberType _eType)
{
	if (m_sessionCount >= 4) return;

	unsigned int size = m_arrSession.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrSession[i].pSession == nullptr)
		{
			m_arrSession[i].pSession = _pSession;
			m_arrSession[i]._eType = _eType;
			m_arrSession[i]._eState = _eType == eMemberType::Owner ? eMemberState::Ready : eMemberState::Wait;
			m_sessionCount++;
			return;
		}
	}
}

// �ڽ��� �������� üũ
// �����̸� �ٸ������ ����
void Room::LeaveSession(Session* _pSession)
{
	unsigned int size = m_arrSession.size();
	int i = 0;
	bool isOwner = false;
	for (; i < size; i++)
	{
		if (m_arrSession[i].pSession == _pSession)
		{
			if(m_arrSession[i]._eType == eMemberType::Owner) 
				isOwner = true;
			m_arrSession[i].pSession = nullptr;
			m_arrSession[i]._eType = eMemberType::None;
			m_arrSession[i]._eState = eMemberState::None;
			m_sessionCount--;
			break;
		}
	}

	if (isOwner)
	{
		for (i = 0; i < size; i++)
		{
			if (m_arrSession[i].pSession == nullptr) continue;
			m_arrSession[i]._eType = eMemberType::Owner;
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
	unsigned int size = m_arrSession.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrSession[i]._eType == eMemberType::Owner)
			return &m_arrSession[i];
	}
	return nullptr;
}

const tMember* Room::GetMemberInfo(Session* _pSession)
{
	size_t size = m_arrSession.size();
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrSession[i].pSession == _pSession)
			return &m_arrSession[i];
	}
	return nullptr;
}

bool Room::IsRoomReady()
{
	size_t size = m_arrSession.size();
	if (m_sessionCount <= 1) return false;
	for (size_t i = 0; i < size; i++)
	{
		if (m_arrSession[i]._eState == eMemberState::Wait)
				return false;
	}
	return true;
}

void Room::SendAll(char* _buffer)
{
	size_t size = m_arrSession.size();
	for (int i = 0; i < size; i++)
	{
		if (m_arrSession[i].pSession == nullptr) continue;
		send(m_arrSession[i].pSession->GetSocket(), _buffer, *(u_short*)_buffer, 0);
	}
}