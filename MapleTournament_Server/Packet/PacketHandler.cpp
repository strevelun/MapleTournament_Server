#include "PacketHandler.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Managers/RoomManager.h"
#include "../Managers/GameManager.h"
#include "../Managers/SkillManager.h"
#include "../Game.h"
#include "../Skill.h"
#include "../SkillHeal.h"
#include "../SkillAttack.h"
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
	std::vector<Session*> vecSession;
	SessionManager::GetInst()->GetVecSession(vecSession);
	u_int size = vecSession.size();
	for(auto& session : vecSession)
	{
		pUser = session->GetUser();
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
	const Member* pMember = nullptr;
	if (pRoom)
	{
		pMember = pRoom->GetMemberInfo(_pSession->GetId());

		char buffer[255];
		ushort count = sizeof(ushort);
		unsigned int roomId = pRoom->GetId();
		unsigned int memberCount = pRoom->GetMemberCount();
		pRoom->LeaveMember(_pSession);

		if (eState == eSessionState::InGame)
		{
			Game* pGame = GameManager::GetInst()->FindGame(roomId); 
			if (pGame)
			{
				//int curPlayerSlot = pGame->GetCurPlayerSlot();
		
				*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateIngameUserLeave;				count += sizeof(ushort);
				*(char*)(buffer + count) = (char)pMember->GetInfo().slotNumber;					count += sizeof(char);
				*(char*)(buffer + count) = (char)pGame->GetCurSkillName(pMember->GetInfo().slotNumber);					count += sizeof(char);
				*(ushort*)buffer = count;
				pRoom->SendAll(buffer);

				if (pGame->GetCurPlayerSlot() == pMember->GetInfo().slotNumber)
				{
					pGame->OnNextTurn();

				}
				pGame->RemovePlayer(pMember->GetInfo().slotNumber);
				if (memberCount <= 2) // 2���� ���¿��� �� �� �̻��� ���� ���� �� ���
				{
					pGame->SendGameOverPacket();
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
			const Member* pNewOwner = pRoom->GetRoomOwner();

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomMemberLeave;				count += sizeof(ushort);
			*(char*)(buffer + count) = pMember->GetInfo().slotNumber;				count += sizeof(char); // ���� ��������
			*(char*)(buffer + count) = pNewOwner->GetInfo().slotNumber;				count += sizeof(char); // ���� ���ο� owner����
			*(ushort*)buffer = count;
			pRoom->SendAll(buffer);

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;				count += sizeof(ushort);
			*(char*)(buffer + count) = (char)eMemberType::Owner;			count += sizeof(char);
			*(ushort*)buffer = count;
			send(pNewOwner->GetSocket(), buffer, count, 0);
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
	pRoom->AddMember(_pSession, eMemberType::Owner);

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
		pRoom->AddMember(_pSession);
		const wchar_t* roomTitle = pRoom->GetRoomTitle();

		// �̹� �濡 �ִ� �������� ������ ������
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;			count += sizeof(ushort);
		memcpy(buffer + count, roomTitle, wcslen(roomTitle) * 2);				count += (ushort)wcslen(roomTitle) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		unsigned int userCount = pRoom->GetMemberCount();
		*(ushort*)(buffer + count) = (ushort)userCount;							count += sizeof(ushort);
		const std::array<Member, 4>& userList = pRoom->GetMemberList();
		
		const wchar_t* nickname = nullptr;
		const Session* s = nullptr;

		size_t size = userList.size();
		for (int i=0; i< size; i++)
		{
			if (!pRoom->IsMemberExist(i))				continue;

			*(char*)(buffer + count) = (char)i;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].GetInfo().characterChoice;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].GetInfo().eType;			count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].GetInfo().eState;				count += sizeof(char);

			pUser = userList[i].GetInfo().pUser;
			nickname = pUser->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		}
		*(ushort*)buffer = count;
		send(clientSocket, buffer, count, 0);

		// ��� ���� �÷��̾��� ������ ��� ��� �����鿡�� (�ڱ��ڽ� ����)
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_NotifyJoinedUser;		count += sizeof(ushort);
		const Member* info = pRoom->GetMemberInfo(_pSession->GetId());
		*(char*)(buffer + count) = (char)info->GetInfo().slotNumber;					count += sizeof(char);
		*(char*)(buffer + count) = (char)info->GetInfo().characterChoice;					count += sizeof(char);
		*(ushort*)(buffer + count) = (ushort)info->GetInfo().eType;				count += sizeof(ushort);
		nickname = _pSession->GetUser()->GetNickname();
		memcpy(buffer + count, nickname, wcslen(nickname) * 2);					count += (ushort)wcslen(nickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(ushort*)buffer = count;												

		pRoom->SendAll(buffer, _pSession);
	}
}

void PacketHandler::C_LeaveRoom(Session* _pSession, char* _packet)
{
	char buffer[255];
	ushort count = sizeof(ushort);
	Room* pRoom = _pSession->GetRoom();
	const Member* pMember = pRoom->GetMemberInfo(_pSession->GetId());
	unsigned int roomId = pRoom->GetId();
	if (pRoom->GetMemberCount() <= 1)
	{
		RoomManager::GetInst()->DeleteRoom(roomId);
	}
	else
	{
		pRoom->LeaveMember(_pSession);
		const Member* pNewOwner = pRoom->GetRoomOwner();

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateRoomMemberLeave;				count += sizeof(ushort);
		*(char*)(buffer + count) = pMember->GetInfo().slotNumber;				count += sizeof(char); // ���� ��������
		*(char*)(buffer + count) = pNewOwner->GetInfo().slotNumber;				count += sizeof(char); // ���� ���ο� owner����
		*(ushort*)buffer = count;
		pRoom->SendAll(buffer);

		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;				count += sizeof(ushort);
		*(char*)(buffer + count) = (char)eMemberType::Owner;			count += sizeof(char);
		*(ushort*)buffer = count;
		send(pNewOwner->GetSocket(), buffer, count, 0);
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
		Player* pPlayer = nullptr;
		const std::array<Member, 4>& memberList = pRoom->GetMemberList();

		int i = 0;
		for (const Member& member : memberList)
		{
			if (!pRoom->IsMemberExist(i++)) continue;

			if (member.GetInfo().eType == eMemberType::Member)
			{
				pRoom->SetMemberState(member.GetSocket(), eMemberState::Wait);
			}

			int xpos, ypos;
			if (member.GetInfo().slotNumber == 0)
			{
				xpos = 0;
				ypos = 0;
			}
			else if (member.GetInfo().slotNumber == 1)
			{
				xpos = 4;
				ypos = 0;
			}
			else if (member.GetInfo().slotNumber == 2)
			{
				xpos = 0;
				ypos = 3;
			}
			else if (member.GetInfo().slotNumber == 3)
			{
				xpos = 4;
				ypos = 3;
			}
			pGame->AddPlayer(member.GetSocket(), member.GetInfo().slotNumber, xpos, ypos);
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

	const Member* member = pRoom->GetMemberInfo(_pSession->GetId());
	eMemberState state = member->GetInfo().eState;
	eMemberState changeState = eMemberState(3 - (int)state);
	pRoom->SetMemberState(_pSession->GetId(), changeState);

	char buffer[255];
	ushort count = sizeof(ushort);		
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserState;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)changeState;					count += sizeof(char);
	*(char*)(buffer + count) = (char)member->GetInfo().slotNumber;			count += sizeof(char);
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
	const std::array<Member, Game::RoomSlotNum>& memberList = pRoom->GetMemberList();
	const Member* tm = pRoom->GetMemberInfo(_pSession->GetId());
	_pSession->ChangeSessionState(eSessionState::InGame);

	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	Player* player = pGame->FindPlayer(tm->GetInfo().slotNumber);
	if (player) player->SetReady(true);

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_InGameReady;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pRoom->GetMemberCount();			count += sizeof(char);
	*(char*)(buffer + count) = (char)tm->GetInfo().slotNumber;			count += sizeof(char);

	int i = 0;
	for (const Member& member : memberList)
	{
		if (!pRoom->IsMemberExist(i++)) continue;

		*(char*)(buffer + count) = (char)member.GetInfo().slotNumber;		count += sizeof(char);
		*(char*)(buffer + count) = (char)member.GetInfo().characterChoice;			count += sizeof(char);
		*(char*)(buffer + count) = (char)HPMax;			count += sizeof(char);
		*(char*)(buffer + count) = (char)MPMax;			count += sizeof(char);

		User* pUser = member.GetInfo().pUser;
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
	std::vector<Session*> vecSession;
	SessionManager::GetInst()->GetVecSession(vecSession);
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
	for(auto& session : vecSession)
	{
		eSessionState eState = session->GetSessionState();
		if (eState == eSessionState::Login) continue;

		loginedUserCount++;
		if (loginedUserCount <= minCount) continue;

		User* pUser = session->GetUser();
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
			Room* pRoom = session->GetRoom();
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
	std::vector<Room*> roomList;
	RoomManager::GetInst()->GetRoomList(roomList);
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

	std::vector<Room*>::iterator iter = roomList.begin();
	std::vector<Room*>::iterator iterEnd = roomList.end();

	std::advance(iter, minCount);

	User* pUser = nullptr;
	const wchar_t* pOwnerNickname = nullptr;

	for (; iter != iterEnd; ++iter, numOfRoom++)
	{
		if (numOfRoom >= roomListSize) break;
		if (numOfRoom >= roomListPageViewCount) break;

		*(u_int*)(buffer + count) = (*iter)->GetId();			count += sizeof(u_int);
		*(ushort*)(buffer + count) = (ushort)(*iter)->GetRoomState();			count += sizeof(ushort);
		const wchar_t* title = (*iter)->GetRoomTitle();
		memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
		const Member* member = (*iter)->GetRoomOwner();
		pUser = member->GetInfo().pUser;
		pOwnerNickname = pUser->GetNickname();
		memcpy(buffer + count, pOwnerNickname, wcslen(pOwnerNickname) * 2);			count += (ushort)wcslen(pOwnerNickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
		*(ushort*)(buffer + count) = (ushort)(*iter)->GetMemberCount();			count += sizeof(ushort);
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
	const Member* member = pRoom->GetMemberInfo(_pSession->GetId());
	if (member->GetInfo().characterChoice == choice) return;

	pRoom->SetMemberChoice(_pSession->GetId(), choice);
	
	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserSlot;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)member->GetInfo().slotNumber;						count += sizeof(char);
	*(char*)(buffer + count) = (char)member->GetInfo().characterChoice;					count += sizeof(char);
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

}

void PacketHandler::C_Skill(Session* _pSession, char* _packet)
{
	eActionType type = eActionType(*(char*)_packet);				_packet += sizeof(char);
	Room* pRoom = _pSession->GetRoom();
	if (!pRoom) return;
	
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	Player* pPlayer = pGame->FindPlayer(_pSession);
	// ���� �Ҹ��Ű��, �Ҹ�� ���� ��Ŷ���� ����
		
	// pGame->SetSkillType(member->slotNumber, type);

	// TODO : �ڵ� �帧 ����

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_Skill;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pPlayer->GetSlot();						count += sizeof(char);
	*(char*)(buffer + count) = (char)type;								count += sizeof(char);

	if (type == eActionType::Move)
	{
		eMoveName name = eMoveName(*(char*)_packet);
		name = pGame->Move(pPlayer->GetSlot(), name);
		*(char*)(buffer + count) = (char)name;							count += sizeof(char);

		if (name != eMoveName::None)
		{
			pGame->CheckPortal(pPlayer->GetSlot());
		}
	}
	else if(type == eActionType::Skill)
	{
		eSkillName name = eSkillName(*(char*)_packet);
		const Skill* pSkill = SkillManager::GetInst()->GetSkill(pPlayer->GetSlot(), name);
		pPlayer->SetMana(pPlayer->GetMana() - pSkill->GetMana());
		pGame->SetSkillName(pPlayer->GetSlot(), name);
		*(char*)(buffer + count) = (char)pPlayer->GetMana();			count += sizeof(char);
		*(char*)(buffer + count) = (char)name;						count += sizeof(char);
		const SkillAttack* pAttack = dynamic_cast<const SkillAttack*>(pSkill);
		if (pAttack)
		{
			const std::vector<std::pair<int, int>>& list = pAttack->GetListCoordinates();
			char* listSize = (char*)(buffer + count);			count += sizeof(char);
			int xpos, ypos, size = 0;
			for (const auto& coor : list)
			{
				xpos = pPlayer->GetXPos() + coor.first;
				ypos = pPlayer->GetYPos() + coor.second;
				if (xpos < 0 || ypos < 0 || xpos >= Game::BoardWidth || ypos >= Game::BoardHeight)
					continue;
				*(char*)(buffer + count) = (char)xpos;			count += sizeof(char);
				*(char*)(buffer + count) = (char)ypos;			count += sizeof(char);
				size++;
			}
			*listSize = (char)size;
		}
		else
		{
			*(char*)(buffer + count) = (char)0;			count += sizeof(char);
		}
	}
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

}

void PacketHandler::C_NextTurn(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	if (!pRoom) return;


	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	if (pGame)
	{
		// ���� ��Ż�� ���� �̵��ϴ� ����� ���� �� ���� NextTurn�� ��� 
		Player* pPlayer = pGame->FindPlayer(_pSession);
		if (pPlayer->IsWaitingForPortal())
		{
			char buffer[255];
			ushort count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateTurn;			count += sizeof(ushort);
			std::vector<eSkillName> skillNameList;
			SkillManager::GetInst()->GetSkillsNotAvailable(pPlayer->GetMana(), skillNameList);
			*(char*)(buffer + count) = (char)skillNameList.size();				count += sizeof(char);
			for (eSkillName name : skillNameList)
			{
				*(char*)(buffer + count) = (char)name;				count += sizeof(char);
			}
			*(ushort*)buffer = count;
			send(pPlayer->GetSocket(), buffer, count, 0);

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_Teleport;			count += sizeof(ushort);
			*(char*)(buffer + count) = (char)pPlayer->GetSlot();			count += sizeof(char);
			*(char*)(buffer + count) = (char)pPlayer->GetXPos();			count += sizeof(char);
			*(char*)(buffer + count) = (char)pPlayer->GetYPos();			count += sizeof(char);
			*(ushort*)buffer = count;
			pGame->SendAll(buffer);

			printf("S_UpdateTurn : (%d, %d, %d)\n", pPlayer->GetSlot(), pPlayer->GetXPos(), pPlayer->GetYPos());

			pPlayer->SetWaitForPortal(false);
			return;
		}

		pGame->OnNextTurn();
	}
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
		const std::array<Member, 4>& userList = pRoom->GetMemberList();
		
		const wchar_t* nickname = nullptr;
		User* user = nullptr;

		size_t size = userList.size();
		for (int i = 0; i < size; i++)
		{
			if (!pRoom->IsMemberExist(i))			continue;

			*(char*)(buffer + count) = (char)i;									count += sizeof(char);
			*(char*)(buffer + count) = (char)userList[i].GetInfo().characterChoice;									count += sizeof(char);
			*(ushort*)(buffer + count) = (ushort)userList[i].GetInfo().eType;			count += sizeof(ushort);
			nickname = userList[i].GetInfo().pUser->GetNickname();
			memcpy(buffer + count, nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		}

		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);

		for (int i = 0; i < size; i++)
		{
			if (!pRoom->IsMemberExist(i))			continue;

			count = sizeof(ushort);
			*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateUserType;			count += sizeof(ushort);
			*(char*)(buffer + count) = (char)userList[i].GetInfo().eType;				count += sizeof(char);
				*(ushort*)buffer = count;
			send(userList[i].GetSocket(), buffer, count, 0);
		}
	}

	// ų ���� �Ź� �κ���� �������� ���������� ���� ���� �ÿ� �ѹ� ������ Ŭ�󿡼� 
	// �α��� �� S_UpdateProfile �ѹ�.
	// �г��� �����ϱ�
	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateProfile;			count += sizeof(ushort);
	User* pUser = _pSession->GetUser();
	if (pUser)
	{
		const wchar_t* myNickname = pUser->GetNickname();
		memcpy(buffer + count, myNickname, wcslen(myNickname) * 2);				count += (ushort)wcslen(myNickname) * 2;
		*(wchar_t*)(buffer + count) = L'\0';								count += 2;
		*(u_int*)(buffer + count) = pUser->GetKillCount();					count += sizeof(u_int);
		*(ushort*)buffer = count;
		send(_pSession->GetSocket(), buffer, count, 0);
	}
}

// ���Ĺ��� �� ���� ���� üũ
void PacketHandler::C_Standby(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	const std::array<Member, Game::RoomSlotNum>& memberList = pRoom->GetMemberList();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());

	Player* pPlayer = pGame->FindPlayer(pRoom->GetMemberInfo(_pSession->GetId())->GetInfo().slotNumber);
	pPlayer->SetStandby(true);

	if (pGame->IsAllStandby())
	{
		int slot = pGame->UpdateNextTurn();

		// S_UpdateTurn�� ���� ������ �������� ��ų�� ������ ���� �����.
		char buffer[255];
		ushort count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateTurn;			count += sizeof(ushort);
		*(char*)(buffer + count) = (char)0;				count += sizeof(char);
		*(ushort*)buffer = count;
		send(memberList[slot].GetSocket(), buffer, count, 0);
	}
}

void PacketHandler::C_CheckHit(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	Player* pPlayer = pGame->FindPlayer(_pSession);
	
	std::vector<Player*> hitPlayerList, deadPlayerList;
	pGame->GetHitResult(pPlayer->GetSlot(), hitPlayerList, deadPlayerList);

	char playerSize = (char)hitPlayerList.size();

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CheckHit;			count += sizeof(ushort);
	*(char*)(buffer + count) = (char)playerSize;									count += sizeof(char);

	// listHitPlayer���� ���� eSkillType�� Shield�� �ֵ��� type�� ������
	for (const auto& player : hitPlayerList)
	{
		*(char*)(buffer + count) = (char)player->GetSlot();							count += sizeof(char);
		*(char*)(buffer + count) = (char)player->GetHP();							count += sizeof(char);
	}

	*(char*)(buffer + count) = (char)deadPlayerList.size();									count += sizeof(char);

	//pPlayer->m_score += deadPlayerList.size();
	pPlayer->SetScore(pPlayer->GetScore() + deadPlayerList.size());

	for (const auto& player : deadPlayerList)
	{
		*(char*)(buffer + count) = (char)player->GetSlot();							count += sizeof(char);
		pGame->RemovePlayerFromBoard(player->GetSlot());
	}
	*(ushort*)buffer = count;
	pGame->SendAll(buffer);
}

void PacketHandler::C_CheckHeal(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());
	Player* pPlayer = pGame->FindPlayer(_pSession);
	
	const Skill* pSkill = SkillManager::GetInst()->GetSkill(0, eSkillName::Heal0); // ���� �� �� ��
	const SkillHeal* pSkillHeal = static_cast<const SkillHeal*>(pSkill);


	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateHeal;			count += sizeof(ushort);

	int healPossible = 1;
	if (pPlayer->GetMana() >= MPMax)
		healPossible = 0;
	else
	{
		int result = pPlayer->GetMana() + pSkillHeal->GetHeal();
		pPlayer->SetMana(result < MPMax ? result : MPMax);
	}

	*(char*)(buffer + count) = (char)healPossible;		count += sizeof(char);
	if (healPossible)
	{
		*(char*)(buffer + count) = (char)pPlayer->GetSlot();							count += sizeof(char);
		*(char*)(buffer + count) = (char)pPlayer->GetMana();		count += sizeof(char);
	}
	*(ushort*)buffer = count;
	pGame->SendAll(buffer);
}

void PacketHandler::C_ExitInGame(Session* _pSession, char* _packet)
{
	Room* pRoom = _pSession->GetRoom();
	Game* pGame = GameManager::GetInst()->FindGame(pRoom->GetId());

	if (pRoom->GetMemberCount() <= 1) return;

	const Member* pMember = pRoom->GetMemberInfo(_pSession->GetId());
	Player* pPlayer = pGame->FindPlayer(_pSession);

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_UpdateIngameUserLeave;				count += sizeof(ushort);
	*(char*)(buffer + count) = (char)pMember->GetInfo().slotNumber;					count += sizeof(char);
	*(char*)(buffer + count) = (char)pGame->GetCurSkillName(pMember->GetInfo().slotNumber);					count += sizeof(char);
	*(ushort*)buffer = count;
	pRoom->SendAll(buffer);

	if (pGame->GetCurPlayerSlot() == pMember->GetInfo().slotNumber)
	{
		pGame->OnNextTurn();

	}
	User* pUser = _pSession->GetUser();
	pUser->AddKillCount(pPlayer->GetScore());
	_pSession->SetRoom(nullptr);
	_pSession->ChangeSessionState(eSessionState::Lobby);
	pGame->RemovePlayer(pMember->GetInfo().slotNumber);
	pRoom->LeaveMember(_pSession);

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_ExitInGame;				count += sizeof(ushort);
	*(ushort*)buffer = count;
	send(_pSession->GetSocket(), buffer, count, 0);
}
