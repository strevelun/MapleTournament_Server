#include "RoomManager.h"

RoomManager* RoomManager::m_pInst = nullptr;

RoomManager::RoomManager()
{
	m_vecInactiveRoomId.resize(ROOM_MAX_SIZE, 0);
	for (int i = ROOM_MAX_SIZE - 1, j = 0; i >= 0; i--, j++)
		m_vecInactiveRoomId[i] = j;
}

RoomManager::~RoomManager()
{
}

Room* RoomManager::CreateRoom(wchar_t* _strTitle)
{
	if (m_count >= ROOM_MAX_SIZE) return nullptr;
	
	unsigned int id = m_vecInactiveRoomId.back();
	m_vecInactiveRoomId.pop_back();

	m_arrRoom[id].Init();
	m_arrRoom[id].SetRoomState(eRoomState::Ready);
	m_arrRoom[id].SetId(id);
	m_arrRoom[id].SetTitle(_strTitle);
	m_count++;

	return &m_arrRoom[id];
}

Room* RoomManager::FindRoom(unsigned int _roomId)
{
	if (m_arrRoom[_roomId].GetRoomState() == eRoomState::None) return nullptr;

	return &m_arrRoom[_roomId];
}

bool RoomManager:: DeleteRoom(unsigned int _roomId)
{
	Room* pRoom = FindRoom(_roomId);
	if (!pRoom)		return false;

	m_arrRoom[_roomId].SetRoomState(eRoomState::None);
	m_vecInactiveRoomId.push_back(_roomId);
	m_count--;
	return true;
}

void RoomManager::GetRoomList(std::vector<Room*>& _vecRoom)
{
	for (int i = 0; i < ROOM_MAX_SIZE; i++)
	{
		if (m_arrRoom[i].GetRoomState() == eRoomState::None) continue;

		_vecRoom.push_back(&m_arrRoom[i]);
	}
}
