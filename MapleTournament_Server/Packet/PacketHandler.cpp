#include "PacketHandler.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Managers/RoomManager.h"
#include "../Network/Room.h"
#include "../Network/User.h"
#include "../Network/Session.h"
#include "../Setting.h"
#include "../Network/Selector.h"

#include <winsock2.h>
#include <iostream>

typedef unsigned short ushort;

void PacketHandler::C_EnterLobby(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	User* pUser = _pSession->GetUser();
	if (!pUser) return;
	const wchar_t* nickname = pUser->GetNickname();
	ushort count;

	// 내가 로그인하기 전에 먼저 로그인했던 세션들 정보 가져오기
	/* 로비에 있는 유저에게만! */
	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	size_t sessionSize = vecSession.size() - 1; // 자기 자신은 제외
	char buffer[255];
	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_SendSessions;		count += sizeof(ushort);
	*(ushort*)(buffer + count) = sessionSize > 8 ? (ushort)8 : (ushort)sessionSize;				count += sizeof(ushort);

	SOCKET s;
	const wchar_t* str;
	for (int i = 0; i < sessionSize; i++)
	{
		s = vecSession[i]->GetSocket();
		if (s == clientSocket) continue;
		eSessionState eState = vecSession[i]->GetSessionState();
		if (eState != eSessionState::Lobby) continue;
		User* pUser = vecSession[i]->GetUser();
		str = pUser->GetNickname();
		memcpy(buffer + count, str, wcslen(str) * 2);							count += (ushort)wcslen(str) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
	}
	*(ushort*)buffer = count;
	send(clientSocket, buffer, count, 0);

	// 룸 개수
	// 아이디 / 상태 / 제목 / 방장 / 인원
	const std::map<unsigned int, Room*>& listRoom = RoomManager::GetInst()->GetRoomList();
	u_int size = listRoom.size();
	if (size >= 1)
	{
		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_SendRooms;		count += sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)size;							count += sizeof(ushort);
		for (auto& room : listRoom)
		{
			*(UINT*)(buffer + count) = room.second->GetId();			count += sizeof(UINT);
			*(ushort*)(buffer + count) = (ushort)room.second->GetRoomState();			count += sizeof(ushort);
			const wchar_t* title = room.second->GetRoomTitle();
			memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			User* pUser = room.second->GetRoomOwner();
			const wchar_t* pOwnerNickname = pUser->GetNickname();
			memcpy(buffer + count, pOwnerNickname, wcslen(pOwnerNickname) * 2);			count += (ushort)wcslen(pOwnerNickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			*(ushort*)(buffer + count) = (ushort)room.second->GetUserCount();			count += sizeof(ushort);
		}
		*(wchar_t*)buffer = count;
		send(clientSocket, buffer, count, 0);
	}
}

void PacketHandler::C_OKLogin(Session* _pSession, char* _packet)
{
	wchar_t* nickname = (wchar_t*)_packet;
	SOCKET clientSocket = _pSession->GetSocket();

	User* pUser = nullptr;

	// 접속중인 세션들을 돌면서 해당 닉네임을 가지는 세션이 있는지 확인
	bool isNicknameUsed = false;
	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	u_int size = vecSession.size();
	for (int i = 0; i < size; i++)
	{
		pUser = vecSession[i]->GetUser();
		if (pUser && wcscmp(pUser->GetNickname(), nickname) == 0)
		{
			isNicknameUsed = true;
			break;
		}
	}

	if (!isNicknameUsed)
	{
		pUser = UserManager::GetInst()->FindUser(nickname);
		if (!pUser)
			pUser = UserManager::GetInst()->CreateUser(nickname);

		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_OKLogin;		count += sizeof(ushort);
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);
		std::wcout << nickname << "님 로그인 완료!" << '\n';

		Session* pSession = SessionManager::GetInst()->FindSession(clientSocket);
		pSession->ChangeSessionState(eSessionState::Lobby);
		pSession->SetUser(pUser);
	}
	else // 닉네임은 이미 존재한 경우
	{
		std::wcout << nickname << "님 로그인 실패!" << '\n';
		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_FailedLogin;		count += sizeof(ushort);
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);
	}
}

void PacketHandler::C_Exit(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	//SessionManager::GetInst()->RemoveSession(clientSocket);
	std::wcout << clientSocket << "로그아웃!" << '\n';

	// S_Exit SendAll loginscene에 없을떄
}

void PacketHandler::C_CreateRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	Room* pRoom = RoomManager::GetInst()->CreateRoom((wchar_t*)_packet);
	Session* pSession = SessionManager::GetInst()->FindSession(clientSocket);
	pSession->ChangeSessionState(eSessionState::WatingRoom);
	User* pMyUser = pSession->GetUser();
	pRoom->AddUser(pMyUser, eMemberType::Owner);
	const wchar_t* nickname = pMyUser->GetNickname();

	std::wcout << clientSocket << "가 방 생성!" << '\n';

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyCreateRoom;		count += sizeof(ushort);
	*(UINT*)(buffer + count) = pRoom->GetId();			count += sizeof(UINT);
	const wchar_t* title = pRoom->GetRoomTitle();
	memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
	*(wchar_t*)(buffer + count) = L'\0';									count += 2;
	memcpy(buffer + count, nickname, wcslen(nickname) * 2);			count += (ushort)wcslen(nickname) * 2;
	*(wchar_t*)(buffer + count) = L'\0';									count += 2;
	*(ushort*)buffer = count;

	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	u_int size = vecSession.size();
	for (int i = 0; i < size; i++)
	{
		eSessionState eState = vecSession[i]->GetSessionState();
		if (vecSession[i]->GetSocket() == clientSocket) continue;
		if (eState != eSessionState::Lobby) continue;
		send(vecSession[i]->GetSocket(), buffer, count, 0);
	}

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CreateRoom;		count += sizeof(ushort);
	*(ushort*)buffer = count;
	send(clientSocket, buffer, count, 0);
}

void PacketHandler::C_Chat(Session* _pSession, char* _packet)
{
	Session* pSession = SessionManager::GetInst()->FindSession(_pSession->GetSocket());
	User* pUser = pSession->GetUser();
	const wchar_t* nickname = pUser->GetNickname();
	ushort count = 0;

	// 패킷사이즈/타입/채팅내용/닉네임
	_packet -= sizeof(ushort);
	*(ushort*)_packet = (ushort)ePacketType::S_Chat;  _packet -= sizeof(ushort);
	ushort packetSize = *(ushort*)_packet;
	memcpy((ushort*)(_packet + packetSize), nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
	*(wchar_t*)(_packet + packetSize + count) = L'\0';													count += 2;
	*(ushort*)_packet = packetSize + count;

	SessionManager::GetInst()->SendAll(_packet, eSessionState::Lobby);
}

void PacketHandler::C_JoinRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	unsigned int roomId = *(unsigned int*)_packet;
	char buffer[255];
	ushort count = sizeof(ushort);

	Room* pRoom = RoomManager::GetInst()->FindRoom(roomId);
	//*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoomFail;		count += sizeof(ushort);
	//*(ushort*)buffer = count;
	
	if (pRoom->GetUserCount() >= 4)
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoomFail;		count += sizeof(ushort);
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);
	}
	else
	{
		Session* pSession = SessionManager::GetInst()->FindSession(clientSocket);
		pSession->ChangeSessionState(eSessionState::WatingRoom);
		User* pUser = pSession->GetUser();
		pRoom->AddUser(pUser);

		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;		count += sizeof(ushort);
		unsigned int userCount = pRoom->GetUserCount();
		*(ushort*)(buffer + count) = (ushort)userCount;						count += sizeof(ushort);
		const std::array<tMember, 4>& userList = pRoom->GetUserList();
		const wchar_t* nickname = nullptr;
		for (int i=0; i<userCount; i++)
		{
			if (userList[i].pUser == nullptr) continue;
			*(ushort*)(buffer + count) = (ushort)i + 1;								count += sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)userList[i]._eType;						count += sizeof(ushort);
			nickname = userList[i].pUser->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';													count += 2;
		}
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);

		// S_NotifyJoinedUser
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyJoinedUser;		count += sizeof(ushort);
		for (int i = 0; i < userCount; i++)
		{
			if (userList[i].pUser == pUser)
			{
				*(ushort*)(buffer + count) = (ushort)i + 1;								count += sizeof(ushort);
				*(ushort*)(buffer + count) = (ushort)userList[i]._eType;						count += sizeof(ushort);
				nickname = userList[i].pUser->GetNickname();
				memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
				*(wchar_t*)(buffer + count) = L'\0';
				*(ushort*)buffer = count;													count += 2;
				break;
			}
		}
		SessionManager::GetInst()->SendAll(buffer, eSessionState::WatingRoom, clientSocket);
	}
}

void PacketHandler::C_LeaveRoom(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	if (pRoom->GetUserCount() <= 1)
	{
		RoomManager::GetInst()->DeleteRoom(pRoom->GetId());
	}
	else
	{
		pRoom->LeaveUser(_pSession->GetUser());
	}

	_pSession->SetRoom(nullptr);
	_pSession->ChangeSessionState(eSessionState::Lobby);
}
