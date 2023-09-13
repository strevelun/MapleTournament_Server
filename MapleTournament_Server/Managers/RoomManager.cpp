#include "RoomManager.h"
#include "../Network/Room.h"

RoomManager* RoomManager::m_inst = nullptr;
unsigned int RoomManager::m_roomId = 0;

RoomManager::RoomManager()
{
}

RoomManager::~RoomManager()
{
}

Room* RoomManager::CreateRoom(wchar_t* _strTitle)
{
	Room* pRoom = new Room(m_roomId++, _strTitle);
	m_vecRoom.push_back(pRoom);
	return pRoom;
}

Room* RoomManager::FindRoom(unsigned int _roomId)
{
	for (auto& room : m_vecRoom)
	{
		if (room->GetId() == _roomId)
			return room;
	}
	return nullptr;
}
