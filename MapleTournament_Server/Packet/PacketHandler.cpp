#include "PacketHandler.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Managers/RoomManager.h"
#include "../Managers/GameManager.h"
#include "../Managers/SkillManager.h"
#include "../Game.h"
#include "../Skill.h"
#include "../SkillHeal.h"
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

	// �������� ���ǵ��� ���鼭 �ش� �г����� ������ ������ �ִ��� Ȯ��
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
		std::wcout << nickname << "�� �α��� �Ϸ�!" << '\n';

		Session* pSession = SessionManager::GetInst()->FindSession(clientSocket);
		pSession->ChangeSessionState(eSessionState::Lobby);
		pSession->SetUser(pUser);
	}
	else
	{
		std::wcout << nickname << "�� �α��� ����!" << '\n';
		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_FailedLogin;		count += sizeof(ushort);
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);
	}
}

void PacketHandler::C_Exit(Session* _pSession, char* _packet)
{
	// WaitingRoom�� �ִ� ���, ���� �濡 �ִ� �������� Update
	eSessionState eState = _pSession->GetSessionState();
	Room* pRoom = _pSession->GetRoom();
	const tMember* pMember = nullptr;
	if (pRoom)
	{
		pMember = pRoom->GetMemberInfo(_pSession);

		char buffer[255];
		ushort count = sizeof(ushort);
		unsigned int roomId = pRoom->GetId();
		unsigned int memberCount = pRoom->GetMemberCount();
		pRoom->LeaveSession(_pSession);

		if (eState == eSessionState::InGame)
		{
			Game* pGame = GameManager::GetInst()->FindGame(roomId); 
			if (pGame)
			{
				int curPlayerSlot = pGame->GetCurPlayerSlot();

		
				*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateIngameUserLeave;				count += sizeof(ushort);
				*(char*)(buffer + count) = (char)curPlayerSlot;					count += sizeof(char);
				*(char*)(buffer + count) = (char)pGame->GetCurSkillType(curPlayerSlot);					count += sizeof(char);
				*(ushort*)buffer = count;
				pRoom->SendAll(buffer);

				if (curPlayerSlot == pMember->slotNumber)
				{
					pGame->OnNextTurn();

				}
				pGame->RemovePlayer(pMember->slotNumber);
				if (memberCount <= 2) // 2���� ���¿��� �� �� �̻��� ���� ���� �� ���
				{
					count = sizeof(ushort);
					*(ushort*)(buffer + count) = (ushort)ePacketType::S_GameOver;				count += sizeof(ushort);
					*(ushort*)buffer = count;
					pRoom->SendAll(buffer);
					GameManager::GetInst()->DeleteGame(roomId);
				}
			}
		}

		if (memberCount <= 1)
		{
			RoomManager::GetInst()->DeleteRoom(roomId);
		}
		else if (eState == eSessionState::WaitingRoom)
		{
			const tMember* pNewOwner = pRoom->GetRoomOwner();
			Session* newOwnerSession = pNewOwner->pSession;

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomMemberLeave;				count += sizeof(ushort);
			*(char*)(buffer + count) = pMember->slotNumber;				count += sizeof(char); // ���� ��������
			*(char*)(buffer + count) = pNewOwner->slotNumber;				count += sizeof(char); // ���� ���ο� owner����
			*(ushort*)buffer = count;
			pRoom->SendAll(buffer);

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;				count += sizeof(ushort);
			*(char*)(buffer + count) = (char)eMemberType::Owner;			count += sizeof(char);
			*(ushort*)buffer = count;
			send(newOwnerSession->GetSocket(), buffer, count, 0);
		}
	}

	// Lobby or Login�� �ִ� ��� RemoveSession
	SOCKET clientSocket = _pSession->GetSocket();
	if (SessionManager::GetInst()->RemoveSession(clientSocket))
	{
		std::wcout << clientSocket << "�α׾ƿ� ����!" << '\n';
	}
	else
		std::wcout << clientSocket << "�α׾ƿ� ����!" << '\n';

	// S_Exit SendAll loginscene�� ������
}

void PacketHandler::C_CreateRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	Room* pRoom = RoomManager::GetInst()->CreateRoom((wchar_t*)_packet);
	User* pUser = _pSession->GetUser();
	_pSession->ChangeSessionState(eSessionState::WaitingRoom);
	_pSession->SetRoom(pRoom);
	pRoom->AddSession(_pSession, eMemberType::Owner);

	std::wcout << clientSocket << "�� �� ����!" << '\n';
	
	const wchar_t* roomTitle = pRoom->GetRoomTitle();
	const wchar_t* nickname = pUser->GetNickname();

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CreateRoom;		count += sizeof(ushort);
	memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
	*(wchar_t*)(buffer + count) = L'\0';								count += 2;
	memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
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
	else if(eState == eSessionState::WaitingRoom)
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
		_pSession->ChangeSessionState(eSessionState::WaitingRoom);
		_pSession->SetRoom(pRoom);
		User* pUser = _pSession->GetUser();
		pRoom->AddSession(_pSession);
		const wchar_t* roomTitle = pRoom->GetRoomTitle();

		// �̹� �濡 �ִ� �������� ������ ������
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;			count += sizeof(ushort);
		memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		unsigned int userCount = pRoom->GetMemberCount();
		*(ushort*)(buffer + count) = (ushort)userCount;							count += sizeof(ushort);
		const std::array<tMember, 4>& userList = pRoom->GetMemberList();
		
		const wchar_t* nickname = nullptr;
		Session* s = nullptr;
		User* user = nullptr;

		size_t size = userList.size();
		for (int i=0; i< size; i++)
		{
			if (userList[i].pSession == nullptr)				continue;

			*(char*)(buffer + count) = (char)i;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].characterChoice;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i]._eType;			count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i]._eState;				count += sizeof(char);

			s = userList[i].pSession;
			user = s->GetUser();
			nickname = user->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		}
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);

		// ��� ���� �÷��̾��� ������ ��� ��� �����鿡�� (�ڱ��ڽ� ����)
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyJoinedUser;		count += sizeof(ushort);
		const tMember* info = pRoom->GetMemberInfo(_pSession);
		*(char*)(buffer + count) = (char)info->slotNumber;					count += sizeof(char);
		*(char*)(buffer + count) = (char)info->characterChoice;					count += sizeof(char);
		*(ushort*)(buffer + count) = (ushort)info->_eType;				count += sizeof(ushort);
		nickname = info->pSession->GetUser()->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);					count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(ushort*)buffer = count;												
		
		// Room::SendAll
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
		*(char*)(buffer + count) = pMember->slotNumber;				count += sizeof(char); // ���� ��������
		*(char*)(buffer + count) = pNewOwner->slotNumber;				count += sizeof(char); // ���� ���ο� owner����
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

		Game* pGame = new Game;
		tPlayer* pPlayer = nullptr;
		const std::array<tMember, 4>& memberList = pRoom->GetMemberList();

		for (const tMember& member : memberList)
		{
			if (member.pSession == nullptr) continue;

			if (member._eType == eMemberType::Member)
			{
				pRoom->SetMemberState(member.pSession, eMemberState::Wait);
			}

			pPlayer = new tPlayer;
			pPlayer->socket = member.pSession->GetSocket();
			pPlayer->slot = member.slotNumber;
			if (member.slotNumber == 0)
			{
				pPlayer->xpos = 0;
				pPlayer->ypos = 0;
			}
			else if (member.slotNumber == 1)
			{
				pPlayer->xpos = 4;
				pPlayer->ypos = 0;
			}
			else if (member.slotNumber == 2)
			{
				pPlayer->xpos = 0;
				pPlayer->ypos = 3;
			}
			else if (member.slotNumber == 3)
			{
				pPlayer->xpos = 4;
				pPlayer->ypos = 3;
			}
			pGame->AddPlayer(pPlayer);
		}

		GameManager::GetInst()->AddGame(pRoom->GetId(), pGame);
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

	if (pRoom->GetRoomState() == eRoomState::InGame) return;

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
	/* �� �ʱ�ȭ �Ϸ� �� ���� ���� ���� ���� ���� */
	Room* pRoom = _pSession->GetRoom();
	const std::array<tMember, Game::RoomSlotNum>& memberList = pRoom->GetMemberList();
	const tMember* tm = pRoom->GetMemberInfo(_pSession);
	_pSession->ChangeSessionState(eSessionState::InGame);

	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	tPlayer* player = pGame->FindPlayer(tm->slotNumber);
	if (player) player->ready = true;

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_InGameReady;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pRoom->GetMemberCount();			count += sizeof(char);
	*(char*)(buffer + count) = (char)tm->slotNumber;			count += sizeof(char);

	for (const tMember& member : memberList)
	{
		if (member.pSession == nullptr) continue;

		*(char*)(buffer + count) = (char)member.slotNumber;		count += sizeof(char);
		*(char*)(buffer + count) = (char)member.characterChoice;			count += sizeof(char);
		*(char*)(buffer + count) = (char)HPMax;			count += sizeof(char);
		*(char*)(buffer + count) = (char)MPMax;			count += sizeof(char);

		User* pUser = member.pSession->GetUser();
		const wchar_t* nickname = pUser->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
	}
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
	
	// ���� �� ��Ŷ�� ������ Ȯ���� �� Game ���� ����
	if (pGame->IsAllReady())
	{
		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_Standby;			count += sizeof(ushort);
		*(ushort*)buffer = count;
		pGame->SendAll(buffer);

		//pGame->SetGameState(eGameState::Choice);
	}
}

void PacketHandler::C_UpdateUserListPage(Session* _pSession, char* _packet)
{
	const int userListPageViewCount = 9;
	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	size_t size = vecSession.size();
	int newPage = *(char*)_packet;

	int minCount = newPage * userListPageViewCount;
	u_int loginedUserCount = SessionManager::GetInst()->GetLoginedUserCount();
	if (loginedUserCount <= minCount)
	{
		newPage = (loginedUserCount - 1) / userListPageViewCount; // �� ��Ŷ�� �޾Ҵٴ� ���� �α����� ������ �ּ� 1��
		minCount = newPage * userListPageViewCount;
	}
	int maxCount = (newPage + 1) * userListPageViewCount;

	// ���� �� 10�� �������̶� 1�������� �� �� ������ �����ְ� �־��µ�, �� ���� ������ �����ϸ� ������ ���� ��� ������ ������ ��ܾ� ��. ���� 5�������� ���� �ִµ� ������ ���� ���� �����ϸ� 0��������.

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserListPage;			count += sizeof(ushort);
	count += sizeof(char);
	count += sizeof(char);

	loginedUserCount = 0;
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
			count += sizeof(u_int);
		}

		if (loginedUserCount >= maxCount)
			break;
	}

	*(char*)(buffer + sizeof(ushort) + sizeof(ushort)) = (char)newPage;
	*(char*)(buffer + sizeof(ushort) + sizeof(ushort) + sizeof(char)) = (char)loginedUserCount - (newPage * userListPageViewCount);
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}

void PacketHandler::C_UpdateRoomListPage(Session* _pSession, char* _packet)
{
	const int roomListPageViewCount = 10;
	const std::map<unsigned int, Room*>& roomList = RoomManager::GetInst()->GetRoomList();
	size_t roomListSize = roomList.size();
	int numOfRoom = 0;

	int newPage = *(char*)_packet;
	int minCount = newPage * roomListPageViewCount;
	if (roomListSize <= minCount)
	{
		newPage = roomListSize > 0 ? (roomListSize - 1) / roomListPageViewCount : 0; 
		minCount = newPage * roomListPageViewCount;
	}
	int maxCount = (newPage + 1) * roomListPageViewCount;

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

	for (; iter != iterEnd; ++iter, numOfRoom++)
	{
		if (numOfRoom >= roomListSize) break;
		if (numOfRoom >= roomListPageViewCount) break;

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

void PacketHandler::C_UpdateUserSlot(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	if (!pRoom) return;

	int choice = *(char*)_packet;
	const tMember* member = pRoom->GetMemberInfo(_pSession);
	if (member->characterChoice == choice) return;

	pRoom->SetMemberChoice(_pSession, choice);
	
	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserSlot;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)member->slotNumber;						count += sizeof(char);
	*(char*)(buffer + count) = (char)member->characterChoice;					count += sizeof(char);
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

}

void PacketHandler::C_Skill(Session* _pSession, char* _packet)
{
	eActionType type = eActionType(*(char*)_packet);				_packet += sizeof(char);
	Room* pRoom = _pSession->GetRoom();
	if (!pRoom) return;
	
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	tPlayer* pPlayer = pGame->FindPlayer(_pSession);
	// ���� �Ҹ��Ű��, �Ҹ�� ���� ��Ŷ���� ����
		
	// pGame->SetSkillType(member->slotNumber, type);

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_Skill;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pPlayer->slot;						count += sizeof(char);
	*(char*)(buffer + count) = (char)type;								count += sizeof(char);

	if (type == eActionType::Move)
	{
		eMoveName name = eMoveName(*(char*)_packet);
		name = pGame->Move(pPlayer->slot, name);
		*(char*)(buffer + count) = (char)name;
	}
	else if(type == eActionType::Skill)
	{
		eSkillName name = eSkillName(*(char*)_packet);
		const Skill* pSkill = SkillManager::GetInst()->GetSkill(name);
		pPlayer->mana -= pSkill->GetMana();
		pGame->SetSkillType(pPlayer->slot, name);
		*(char*)(buffer + count) = (char)pPlayer->mana;			count += sizeof(char);
		*(char*)(buffer + count) = (char)name;
	}
	count += sizeof(char);
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

}

void PacketHandler::C_NextTurn(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	if (!pRoom) return;

	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	if(pGame)
		pGame->OnNextTurn();
}

void PacketHandler::C_GameOver(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	pRoom->SetRoomState(eRoomState::Ready);
	_pSession->ChangeSessionState(eSessionState::WaitingRoom);
}

void PacketHandler::C_LobbyInit(Session* _pSession, char* _packet)
{
	eSessionState state = _pSession->GetSessionState();
	Room* pRoom = _pSession->GetRoom();

	char buffer[255];
	ushort count = sizeof(ushort);

	if (state == eSessionState::WaitingRoom)
	{
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_GameOverSceneChange;			count += sizeof(ushort);

		// �̹� �濡 �ִ� �������� ������ ������
		const wchar_t* roomTitle = pRoom->GetRoomTitle();
		memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		unsigned int userCount = pRoom->GetMemberCount();
		*(ushort*)(buffer + count) = (ushort)userCount;							count += sizeof(ushort);
		const std::array<tMember, 4>& userList = pRoom->GetMemberList();
		
		const wchar_t* nickname = nullptr;
		Session* s = nullptr;
		User* user = nullptr;

		size_t size = userList.size();
		for (int i = 0; i < size; i++)
		{
			if (userList[i].pSession == nullptr)				continue;

			*(char*)(buffer + count) = (char)i;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].characterChoice;									count += sizeof(char);
			*(ushort*)(buffer + count) = (ushort)userList[i]._eType;			count += sizeof(ushort);
			s = userList[i].pSession;
			user = s->GetUser();
			nickname = user->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		}

		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);

		for (int i = 0; i < size; i++)
		{
			if (userList[i].pSession == nullptr)				continue;

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;			count += sizeof(ushort);
			*(char*)(buffer + count) = (char)userList[i]._eType;				count += sizeof(char);
				*(ushort*)buffer = count;
			send(userList[i].pSession->GetSocket(), buffer, count, 0);
		}
	}

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateProfile;			count += sizeof(ushort);
	User* pUser = _pSession->GetUser();
	if (pUser)
	{
		const wchar_t* myNickname = pUser->GetNickname();
		memcpy(buffer + count, myNickname, wcslen(myNickname) * 2);				count += (ushort)wcslen(myNickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(u_int*)(buffer + count) = pUser->GetHitCount();					count += sizeof(u_int);
		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);
	}
}

// ���Ĺ��� �� ���� ���� üũ
void PacketHandler::C_Standby(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	const std::array<tMember, Game::RoomSlotNum>& memberList = pRoom->GetMemberList();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());

	tPlayer* pPlayer = pGame->FindPlayer(pRoom->GetMemberInfo(_pSession)->slotNumber);
	pPlayer->standby = true;

	if (pGame->IsAllStandby())
	{
		int slot = pGame->UpdateNextTurn();

		// S_UpdateTurn�� ���� ������ �������� ��ų�� ������ ���� �����.
		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateTurn;			count += sizeof(ushort);
		*(char*)(buffer + count) = (char)0;				count += sizeof(char);
		*(ushort*)buffer = count;
		send(memberList[slot].pSession->GetSocket(), buffer, count, 0);
	}
}

void PacketHandler::C_CheckHit(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	tPlayer* pPlayer = pGame->FindPlayer(_pSession);
	
	std::list<tPlayer*> hitPlayerList;
	pGame->GetHitPlayerList(pPlayer->slot, hitPlayerList);

	char playerSize = (char)hitPlayerList.size();

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CheckHit;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)playerSize;									count += sizeof(char);

	// listHitPlayer���� ���� eSkillType�� Shield�� �ֵ��� type�� ������
	for (const auto& player : hitPlayerList)
	{
		if (player->hp <= 0)
		{
			player->alive = false;
			player->hp = 0;
		}

		*(char*)(buffer + count) = (char)player->slot;							count += sizeof(char);
		*(char*)(buffer + count) = (char)player->hp;							count += sizeof(char);
		*(char*)(buffer + count) = (char)player->_eSkillName;			count += sizeof(char);
		printf("C_CheckHit : %d, %d, %d\n", player->slot, player->score, (int)player->_eSkillName);
	}
	*(ushort*)buffer = count;
	pGame->SendAll(buffer);
}

void PacketHandler::C_CheckHeal(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	tPlayer* pPlayer = pGame->FindPlayer(_pSession);
	
	const Skill* pSkill = SkillManager::GetInst()->GetSkill(eSkillName::Heal0); // ���� �� �� ��
	const SkillHeal* pSkillHeal = static_cast<const SkillHeal*>(pSkill);


	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateHeal;			count += sizeof(ushort);

	int healPossible = 1;
	if (pPlayer->mana >= MPMax)
		healPossible = 0;
	else
	{
		int result = pPlayer->mana + pSkillHeal->GetHeal();
		pPlayer->mana = result < MPMax ? result : MPMax;
	}

	*(char*)(buffer + count) = (char)healPossible;		count += sizeof(char);
	if (healPossible)
	{
		*(char*)(buffer + count) = (char)pPlayer->slot;							count += sizeof(char);
		*(char*)(buffer + count) = (char)pPlayer->mana;		count += sizeof(char);
	}
	*(ushort*)buffer = count;
	pGame->SendAll(buffer);
}
