#include "PacketHandler.h"
#include "../Managers/SessionManager.h"
#include "../Managers/UserManager.h"
#include "../Managers/RoomManager.h"
#include "../Network/Room.h"
#include "../Network/User.h"
#include "../Network/Session.h"
#include "../Setting.h"
#include "../Network/Selector.h"

#include <iostream>

typedef unsigned short ushort;

void PacketHandler::C_Enter(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	User* pUser = _pSession->GetUser();
	if (!pUser) return;
	const wchar_t* nickname = pUser->GetNickname();
	ushort count;

	// ���� �α����ϱ� ���� ���� �α����ߴ� ���ǵ� ���� ��������
	/* �κ�&�����÷뿡 �ִ� �������Ը�! */
	const std::vector<Session*>& vecSession = SessionManager::GetInst()->GetVecSession();
	size_t sessionSize = vecSession.size() - 1; // �ڱ� �ڽ��� ����
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
		if (eState != eSessionState::Lobby && eState != eSessionState::WatingRoom) continue;
		User* pUser = vecSession[i]->GetUser();
		str = pUser->GetNickname();
		memcpy(buffer + count, str, wcslen(str) * 2);							count += (ushort)wcslen(str) * 2;
		*(wchar_t*)(buffer + count) = L'\0';									count += 2;
	}
	*(ushort*)buffer = count;
	send(clientSocket, buffer, count, 0);

	// �� ����
	// ���̵� / ���� / ���� / ���� / �ο�
	const std::vector<Room*>& listRoom = RoomManager::GetInst()->GetRoomList();
	u_int size = listRoom.size();
	if (size >= 1)
	{
		count = sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)ePacketType::S_SendRooms;		count += sizeof(ushort);
		*(ushort*)(buffer + count) = (ushort)size;							count += sizeof(ushort);
		for (int i = 0; i < size; i++)
		{
			*(UINT*)(buffer + count) = listRoom[i]->GetId();			count += sizeof(UINT);
			*(ushort*)(buffer + count) = (ushort)listRoom[i]->GetRoomState();			count += sizeof(ushort);
			const wchar_t* title = listRoom[i]->GetRoomTitle();
			memcpy(buffer + count, title, wcslen(title) * 2);			count += (ushort)wcslen(title) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			User* pUser = listRoom[i]->GetFirstUser();
			const wchar_t* pOwnerNickname = pUser->GetNickname();
			memcpy(buffer + count, pOwnerNickname, wcslen(pOwnerNickname) * 2);			count += (ushort)wcslen(pOwnerNickname) * 2;
			*(wchar_t*)(buffer + count) = L'\0';									count += 2;
			*(ushort*)(buffer + count) = (ushort)listRoom[i]->GetUserCount();			count += sizeof(ushort);
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
	else // �г����� �̹� ������ ���
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
	SOCKET clientSocket = _pSession->GetSocket();
	//SessionManager::GetInst()->RemoveSession(clientSocket);
	std::wcout << clientSocket << "�α׾ƿ�!" << '\n';

	// S_Exit SendAll loginscene�� ������
}

void PacketHandler::C_CreateRoom(Session* _pSession, char* _packet)
{
	SOCKET clientSocket = _pSession->GetSocket();
	Room* pRoom = RoomManager::GetInst()->CreateRoom((wchar_t*)_packet);
	Session* pSession = SessionManager::GetInst()->FindSession(clientSocket);
	User* pMyUser = pSession->GetUser();
	pRoom->AddUser(pMyUser);
	const wchar_t* nickname = pMyUser->GetNickname();

	std::wcout << clientSocket << "�� �� ����!" << '\n';

	char buffer[255];
	ushort count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_CreateRoom;		count += sizeof(ushort);
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
		if (eState != eSessionState::Lobby && eState != eSessionState::WatingRoom) continue;
		send(vecSession[i]->GetSocket(), buffer, count, 0);
	}

	count = sizeof(ushort);
	*(ushort*)(buffer + count) = (ushort)ePacketType::S_JoinRoom;		count += sizeof(ushort);
	*(ushort*)buffer = count;
	send(clientSocket, buffer, count, 0);
}

void PacketHandler::C_Chat(Session* _pSession, char* _packet)
{
	Session* pSession = SessionManager::GetInst()->FindSession(_pSession->GetSocket());
	User* pUser = pSession->GetUser();
	const wchar_t* nickname = pUser->GetNickname();
	ushort count = 0;

	// ��Ŷ������/Ÿ��/ä�ó���/�г���
	_packet -= sizeof(ushort);
	*(ushort*)_packet = (ushort)ePacketType::S_Chat;  _packet -= sizeof(ushort);
	ushort packetSize = *(ushort*)_packet;
	memcpy((ushort*)(_packet + packetSize), nickname, wcslen(nickname) * 2);				count += (ushort)wcslen(nickname) * 2;
	*(wchar_t*)(_packet + packetSize + count) = L'\0';													count += 2;
	*(ushort*)_packet = packetSize + count;

	SessionManager::GetInst()->SendAll(_packet);
}

void PacketHandler::C_JoinRoom(Session* _pSession, char* _packet)
{
	ushort socket = *(ushort*)_packet;					_packet += sizeof(ushort);
	unsigned int roomId = *(unsigned int*)_packet;

	Room* pRoom = RoomManager::GetInst()->FindRoom(roomId);
	//UserManager::GetInst()->
	//pRoom->AddUser()

}
