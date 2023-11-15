#pragma once

#include <map>
#include <array>
#include <vector>

#include "../Defines.h"

#include "../Network/Room.h"

class RoomManager
{
private:
	unsigned int m_count = 0;

	std::vector<unsigned int> m_vecInactiveRoomId;
	std::array<Room, ROOM_MAX_SIZE> m_arrRoom;

public:
	Room* CreateRoom(wchar_t* _strTitle);
	Room* FindRoom(unsigned int _roomId);
	bool DeleteRoom(unsigned int _roomId);

	void GetRoomList(std::vector<Room*>& _vecRoom);

	SINGLETON(RoomManager)
};

