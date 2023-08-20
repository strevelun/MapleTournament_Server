#include "PacketHandler.h"
#include "../UserManager.h"
#include "../User.h"
#include "FailedLoginPacket.h"
#include "OKLoginPacket.h"

#include <iostream>

PacketHandler::PacketHandler()
{
}

PacketHandler::~PacketHandler()
{
}

void PacketHandler::C_Enter(char* _packet)
{
	// 문자열 뒤에는 항상 널문자가 오도록 해서 strlen으로 길이 읽을 수 있도록
	uint16_t id = *(uint16_t*)_packet;					_packet += sizeof(uint16_t);
	uint32_t nicknameSize = wcslen((wchar_t*)_packet);
	wchar_t* nickname = new wchar_t[(nicknameSize * 2) + 2];
	memcpy(nickname, _packet, (nicknameSize * 2) + 2);
	std::wstring strNickname = nickname;

	User* pUser = UserManager::GetInst()->GetUser(id);
	if (pUser == nullptr)
	{
		printf("C_Enter 처리 도중 에러! : %d는 찾을 수 없는 id입니다.\n", id);
		return;
	}

	User* pOtherUser = UserManager::GetInst()->FindUserByNickname(pUser->GetNickname());
	if(!pOtherUser) 
		pUser->SetNickname(strNickname);
	
	if (!pOtherUser || pOtherUser->GetId() == pUser->GetId())
	{
		OKLoginPacket p;
		send((SOCKET)pUser->GetId(), p.GetPacketBuffer(), p.GetPacketSize(), 0);
		std::wcout << strNickname << "님 로그인 완료!" << '\n';
	}
	else
	{
		FailedLoginPacket p;
		send((SOCKET)pUser->GetId(), p.GetPacketBuffer(), p.GetPacketSize(), 0);
	}
}
