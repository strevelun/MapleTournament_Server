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

		Session* pSession = nullptr;
		User* pUser = nullptr;
		const wchar_t* pOwnerNickname = nullptr;
		for (auto& room : listRoom)
		{
			*(UINT*)(buffer + count) = room.second->GetId();			count += sizeof(UINT);
			*(ushort*)(buffer + count) = (ushort)room.second->GetRoomState();			count += sizeof(ushort);
			const wchar_t* title = room.second->GetRoomTitle();
			memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			const tMember* member = room.second->GetRoomOwner();
			pUser = member->pSession->GetUser();
			pOwnerNickname = pUser->GetNickname();
			memcpy(buffer + count, pOwnerNickname, wcslen(pOwnerNickname) * 2);			count += (ushort)wcslen(pOwnerNickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			*(ushort*)(buffer + count) = (ushort)room.second->GetSessionCount();			count += sizeof(ushort);
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

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_EnterOtherUser;		count += sizeof(ushort);
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(ushort*)buffer = count;
		SessionManager::GetInst()->SendAll(buffer, eSessionState::Lobby, clientSocket);
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
	_pSession->ChangeSessionState(eSessionState::WatingRoom);
	_pSession->SetRoom(pRoom);
	User* pMyUser = _pSession->GetUser();
	pRoom->AddSession(_pSession, eMemberType::Owner);
	const wchar_t* nickname = pMyUser->GetNickname();

	std::wcout << clientSocket << "가 방 생성!" << '\n';

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyCreateRoom;		count += sizeof(ushort);
	*(UINT*)(buffer + count) = pRoom->GetId();								count += sizeof(UINT);
	const wchar_t* title = pRoom->GetRoomTitle();
	memcpy(buffer + count, title, wcslen(title) * 2);						count += (ushort)wcslen(title) * 2;
	*(wchar_t*)(buffer + count) = L'\0';									count += 2;
	memcpy(buffer + count, nickname, wcslen(nickname) * 2);					count += (ushort)wcslen(nickname) * 2;
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
	User* pUser = _pSession->GetUser();
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
	
	if (pRoom->GetSessionCount() >= 4 || pRoom->GetRoomState() == eRoomState::InGame)
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

		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;			count += sizeof(ushort);
		unsigned int userCount = pRoom->GetSessionCount();
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

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateLobbyRoomMemberCount;		count += sizeof(ushort);
		*(unsigned int*)(buffer + count) = roomId;				count += sizeof(unsigned int);
		*(char*)(buffer + count) = (char)pRoom->GetSessionCount();			count += sizeof(char);
		*(ushort*)buffer = count;
		SessionManager::GetInst()->SendAll(buffer, eSessionState::Lobby);
	}
}

void PacketHandler::C_LeaveRoom(Session* _pSession, char* _packet)
{
	
	char buffer[255];
	ushort count = sizeof(ushort);
	Room* pRoom = _pSession->GetRoom();
	const tMember* pMember = pRoom->GetMemberInfo(_pSession);
	unsigned int roomId = pRoom->GetId();
	if (pRoom->GetSessionCount() <= 1)
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_LeaveRoom;				count += sizeof(ushort);
		*(unsigned int*)(buffer + count) = roomId;				count += sizeof(unsigned int);
		*(ushort*)buffer = count;

		SessionManager::GetInst()->SendAll(buffer, eSessionState::Lobby);
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

		// 방장 / 인원수
		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateLobbyRoomList;				count += sizeof(ushort);
		*(unsigned int*)(buffer + count) = roomId;				count += sizeof(unsigned int);
		User* pUser = newOwnerSession->GetUser();
		const wchar_t* nickname = pUser->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(char*)(buffer + count) = (char)pRoom->GetSessionCount();			count += sizeof(char);
		*(ushort*)buffer = count;
		SessionManager::GetInst()->SendAll(buffer, eSessionState::Lobby);
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