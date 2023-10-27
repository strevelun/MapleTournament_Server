#pragma once

#include <map>

#include "../Defines.h"

class Room;

class RoomManager
{
private:
	static unsigned int m_roomId;

	std::map<unsigned int, Room*> m_mapRoom;

public:
	Room* CreateRoom(wchar_t* _strTitle);
	Room* FindRoom(unsigned int _roomId);
	bool DeleteRoom(unsigned int _roomId);

	const std::map<unsigned int, Room*>& GetRoomList() const { return m_mapRoom; }

	SINGLETON(RoomManager)
};

