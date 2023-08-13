#include "PacketHandler.h"
#include "../UserManager.h"
#include "../User.h"

#include <cstdio>

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
	uint32_t nicknameSize = (uint32_t)strlen(_packet);
	wchar_t* nickname = new wchar_t(nicknameSize);
	memcpy(nickname, _packet, nicknameSize);
	User* pUser = UserManager::GetInst()->GetUser(id);
	if (pUser == nullptr)
	{
		printf("C_Enter 처리 도중 에러! : %d는 찾을 수 없는 id입니다.\n", id);
		return;
	}
	pUser->SetNickname(nickname);
}
