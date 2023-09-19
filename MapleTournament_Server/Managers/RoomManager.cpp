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
	Room* pRoom = new Room(m_roomId, _strTitle);
	m_mapRoom.insert({m_roomId++, pRoom}); 
	// std::make_pair vs {}
	// std::vector[i] vs .at()

	return pRoom;
}

Room* RoomManager::FindRoom(unsigned int _roomId)
{
	std::map<unsigned int, Room*>::iterator iter = m_mapRoom.find(_roomId);
	if (iter != m_mapRoom.end())	return iter->second;
	


	return nullptr;
}

bool RoomManager::DeleteRoom(unsigned int _roomId)
{
	Room* pRoom = FindRoom(_roomId);
	if (!pRoom)		return false;

	delete pRoom;
	m_mapRoom.erase(_roomId);

	return false;
}
