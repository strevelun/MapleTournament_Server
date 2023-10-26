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
#include <cassert>

typedef unsigned short ushort;

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
	else
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

	// WaitingRoom에 있는 경우, 같은 방에 있는 유저에게 Update
	if (_pSession->GetSessionState() == eSessionState::WatingRoom)
	{
		char buffer[255];
		ushort count = sizeof(ushort);
		Room* pRoom = _pSession->GetRoom();
		const tMember* pMember = pRoom->GetMemberInfo(_pSession);
		unsigned int roomId = pRoom->GetId();
		if (pRoom->GetMemberCount() <= 1)
		{
			RoomManager::GetInst()->DeleteRoom(roomId);
		}
		else
		{
			pRoom->LeaveSession(_pSession);
			const tMember* pNewOwner = pRoom->GetRoomOwner();
			Session* newOwnerSession = pNewOwner->pSession;

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomMemberLeave;				count += sizeof(ushort);
			*(char*)(buffer + count) = pMember->slotNumber;				count += sizeof(char); // 누가 나갔는지
			*(char*)(buffer + count) = pNewOwner->slotNumber;				count += sizeof(char); // 누가 새로운 owner인지
			*(ushort*)buffer = count;
			pRoom->SendAll(buffer);

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;				count += sizeof(ushort);
			*(char*)(buffer + count) = (char)eMemberType::Owner;			count += sizeof(char);
			*(ushort*)buffer = count;
			send(newOwnerSession->GetSocket(), buffer, count, 0);
		}
	}

	// Lobby or Login에 있는 경우 RemoveSession
	SOCKET clientSocket = _pSession->GetSocket();
	if(SessionManager::GetInst()->RemoveSession(clientSocket))
		std::wcout << clientSocket << "로그아웃 성공!" << '\n';
	else
		std::wcout << clientSocket << "로그아웃 실패!" << '\n';

	// S_Exit SendAll loginscene에 없을떄
}

void PacketHandler::C_CreateRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	Room* pRoom = RoomManager::GetInst()->CreateRoom((wchar_t*)_packet);
	_pSession->ChangeSessionState(eSessionState::WatingRoom);
	_pSession->SetRoom(pRoom);
	pRoom->AddSession(_pSession, eMemberType::Owner);

	std::wcout << clientSocket << "가 방 생성!" << '\n';
	
	const wchar_t* roomTitle = pRoom->GetRoomTitle();

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CreateRoom;		count += sizeof(ushort);
	memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
	*(wchar_t*)(buffer + count) = L'\0';								count += 2;
	*(ushort*)buffer = count;
	send(clientSocket, buffer, count, 0);
}

void PacketHandler::C_Chat(Session* _pSession, char* _packet)
{
	User* pUser = _pSession->GetUser();
	const wchar_t* nickname = pUser->GetNickname();
	ushort count = 0;

	_packet -= sizeof(ushort);
	*(ushort*)_packet = (ushort)ePacketType::S_Chat;  _packet -= sizeof(ushort);
	ushort packetSize = *(ushort*)_packet;
	memcpy((ushort*)(_packet + packetSize), nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
	*(wchar_t*)(_packet + packetSize + count) = L'\0';													count += 2;
	*(ushort*)_packet = packetSize + count;

	eSessionState eState = _pSession->GetSessionState();
	if (eState == eSessionState::Lobby)
	{
		SessionManager::GetInst()->SendAll(_packet, eSessionState::Lobby);
	}
	else if(eState == eSessionState::WatingRoom)
	{
		Room* pRoom = _pSession->GetRoom();
		pRoom->SendAll(_packet);
	}
}

void PacketHandler::C_JoinRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	unsigned int roomId = *(unsigned int*)_packet;
	char buffer[255];
	ushort count = sizeof(ushort);

	Room* pRoom = RoomManager::GetInst()->FindRoom(roomId);
	if (!pRoom) return;

	if (pRoom->GetMemberCount() >= 4 || pRoom->GetRoomState() == eRoomState::InGame)
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoomFail;		count += sizeof(ushort);
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);
	}
	else
	{
		_pSession->ChangeSessionState(eSessionState::WatingRoom);
		_pSession->SetRoom(pRoom);
		User* pUser = _pSession->GetUser();
		pRoom->AddSession(_pSession);
		const wchar_t* roomTitle = pRoom->GetRoomTitle();

		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;			count += sizeof(ushort);
		memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		unsigned int userCount = pRoom->GetMemberCount();
		*(ushort*)(buffer + count) = (ushort)userCount;							count += sizeof(ushort);
		const std::array<tMember, 4>& userList = pRoom->GetMemberList();
		const wchar_t* nickname = nullptr;
		size_t size = userList.size();
		Session* s = nullptr;
		User* user = nullptr;
		for (int i=0; i< size; i++)
		{
			if (userList[i].pSession == nullptr)				continue;
			*(char*)(buffer + count) = (char)i;									count += sizeof(char);
			*(ushort*)(buffer + count) = (ushort)userList[i]._eType;			count += sizeof(ushort);
			s = userList[i].pSession;
			user = s->GetUser();
			nickname = user->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		}
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);

		// 방금 들어온 플레이어는 어느 슬롯에 존재하는지 
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyJoinedUser;		count += sizeof(ushort);
		const tMember* info = pRoom->GetMemberInfo(_pSession);
		*(char*)(buffer + count) = (char)info->slotNumber;					count += sizeof(char);
		*(ushort*)(buffer + count) = (ushort)info->_eType;				count += sizeof(ushort);
		nickname = info->pSession->GetUser()->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);					count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(ushort*)buffer = count;												
		
		for (int i = 0; i < size; i++)
		{
			if (userList[i].pSession == nullptr) continue;
			if (userList[i].pSession == _pSession) continue;
			send(userList[i].pSession->GetSocket(), buffer, count, 0);
		}
	}
}

void PacketHandler::C_LeaveRoom(Session* _pSession, char* _packet)
{
	char buffer[255];
	ushort count = sizeof(ushort);
	Room* pRoom = _pSession->GetRoom();
	const tMember* pMember = pRoom->GetMemberInfo(_pSession);
	unsigned int roomId = pRoom->GetId();
	if (pRoom->GetMemberCount() <= 1)
	{
		RoomManager::GetInst()->DeleteRoom(roomId);
	}
	else
	{
		pRoom->LeaveSession(_pSession);
		const tMember* pNewOwner = pRoom->GetRoomOwner();
		Session* newOwnerSession = pNewOwner->pSession;

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomMemberLeave;				count += sizeof(ushort);
		*(char*)(buffer + count) = pMember->slotNumber;				count += sizeof(char); // 누가 나갔는지
		*(char*)(buffer + count) = pNewOwner->slotNumber;				count += sizeof(char); // 누가 새로운 owner인지
		*(ushort*)buffer = count;
		pRoom->SendAll(buffer);

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;				count += sizeof(ushort);
		*(char*)(buffer + count) = (char)eMemberType::Owner;			count += sizeof(char);
		*(ushort*)buffer = count;
		send(newOwnerSession->GetSocket(), buffer, count, 0);
	}

	_pSession->SetRoom(nullptr);
	_pSession->ChangeSessionState(eSessionState::Lobby);

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_EnterLobby;				count += sizeof(ushort);
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}

void PacketHandler::C_CheckRoomReady(Session* _pSession, char* _packet)
{
	char buffer[255];
	ushort count = sizeof(ushort);

	Room* pRoom = _pSession->GetRoom();
	if (pRoom->IsRoomReady())
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_CheckRoomReadyOK;
		count += sizeof(ushort);
		*(ushort*)buffer = count;
		pRoom->SendAll(buffer);

		pRoom->SetRoomState(eRoomState::InGame);
	}
	else
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_CheckRoomReadyFail;
		count += sizeof(ushort);
		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);
	}
}

void PacketHandler::C_UserRoomReady(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	const tMember* member = pRoom->GetMemberInfo(_pSession);
	eMemberState state = member->_eState;
	eMemberState changeState = eMemberState(3 - (int)state);
	pRoom->SetMemberState(_pSession, changeState);

	char buffer[255];
	ushort count = sizeof(ushort);		
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserState;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)changeState;					count += sizeof(char);
	*(char*)(buffer + count) = (char)member->slotNumber;			count += sizeof(char);
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateWaitingRoomBtn;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)changeState;					count += sizeof(char);
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}

void PacketHandler::C_InGameReady(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	const std::array<tMember, 4>& memberList = pRoom->GetMemberList();

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_InGameReady;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pRoom->GetMemberCount();			count += sizeof(char);
	*(char*)(buffer + count) = (char)pRoom->GetMemberInfo(_pSession)->slotNumber;			count += sizeof(char);

	for (const tMember& member : memberList)
	{
		if (member.pSession == nullptr) continue;
		*(char*)(buffer + count) = member.slotNumber;		count += sizeof(char);
		User* pUser = member.pSession->GetUser();
		const wchar_t* nickname = pUser->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
	}
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}

void PacketHandler::C_UpdateUserListPage(Session* _pSession, char* _packet)
{
	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	int newPage = *(char*)_packet;
	int minCount = newPage * 9;
	int maxCount = (newPage + 1) * 9;

	int loginedUserCount = 0;

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserListPage;			count += sizeof(ushort);
	count += sizeof(char);
	*(char*)(buffer + count) = (char)newPage;						count += sizeof(char);

	size_t size = vecSession.size();
	for (size_t i = 0; i < size; i++)
	{
		eSessionState eState = vecSession[i]->GetSessionState();
		if (eState == eSessionState::Login) continue;

		loginedUserCount++;
		if (loginedUserCount <= minCount) continue;

		User* pUser = vecSession[i]->GetUser();
		const wchar_t* nickname = pUser->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(char*)(buffer + count) = (char)eState;							count += sizeof(char);
		if (eState == eSessionState::Lobby)
		{
			*(char*)(buffer + count) = (char)eState;							count += sizeof(char);
		}
		else
		{
			Room* pRoom = vecSession[i]->GetRoom();
			if(pRoom) 
				*(u_int*)(buffer + count) = pRoom->GetId();
			count += sizeof(char);
		}

		if (loginedUserCount >= maxCount)
			break;
	}
	*(char*)(buffer + sizeof(ushort) + sizeof(ushort)) = (char)loginedUserCount - minCount;
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}

void PacketHandler::C_UpdateRoomListPage(Session* _pSession, char* _packet)
{
	const std::map<unsigned int, Room*>& roomList = RoomManager::GetInst()->GetRoomList();
	size_t roomListSize = roomList.size();
	int numOfRoom = 0;

	int newPage = *(char*)_packet;
	int minCount = newPage * 10;
	int maxCount = (newPage + 1) * 10;

	if (roomListSize > 0 && roomListSize <= minCount) return;

	char buffer[1000];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomListPage;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)newPage;						count += sizeof(char);
	if (roomListSize == 0)
	{
		*(char*)(buffer + count) = (char)0;						count += sizeof(char);
		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);
		return;
	}
	int tempCount = count;
	count += sizeof(char);

	std::map<unsigned int, Room*>::const_iterator iter = roomList.begin();
	std::map<unsigned int, Room*>::const_iterator iterEnd = roomList.end();

	std::advance(iter, minCount);

	User* pUser = nullptr;
	const wchar_t* pOwnerNickname = nullptr;

	for (; iter != iterEnd; iter++, numOfRoom++)
	{
		if (numOfRoom >= roomListSize) break;

		*(u_int*)(buffer + count) = iter->second->GetId();			count += sizeof(u_int);
		*(ushort*)(buffer + count) = (ushort)iter->second->GetRoomState();			count += sizeof(ushort);
		const wchar_t* title = iter->second->GetRoomTitle();
		memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
		const tMember* member = iter->second->GetRoomOwner();
		pUser = member->pSession->GetUser();
		pOwnerNickname = pUser->GetNickname();
		memcpy(buffer + count, pOwnerNickname, wcslen(pOwnerNickname) * 2);			count += (ushort)wcslen(pOwnerNickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
		*(ushort*)(buffer + count) = (ushort)iter->second->GetMemberCount();			count += sizeof(ushort);
	}	

	*(char*)(buffer + tempCount) = (char)numOfRoom;
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}
