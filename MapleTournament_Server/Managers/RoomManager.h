#pragma once

#include <vector>

class Room;

class RoomManager
{
private:
	static RoomManager* m_inst;

	static unsigned int m_roomId;

	std::vector<Room*> m_vecRoom;

	RoomManager();
	~RoomManager();

public:
	static RoomManager* GetInst()
	{
		if (!m_inst)
			m_inst = new RoomManager;
		return m_inst;
	}

	static void DestroyInst()
	{
		if (m_inst)
			delete m_inst;
		m_inst = nullptr;
	}

	Room* CreateRoom(wchar_t* _strTitle);
	Room* FindRoom(unsigned int _roomId);

	const std::vector<Room*>& GetRoomList() const { return m_vecRoom; }
};

