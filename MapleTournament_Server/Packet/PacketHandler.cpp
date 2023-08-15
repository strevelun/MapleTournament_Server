#include "PacketHandler.h"
#include "../UserManager.h"
#include "../User.h"

#include <iostream>

PacketHandler::PacketHandler()
{
}

PacketHandler::~PacketHandler()
{
}

void PacketHandler::C_Enter(char* _packet)
{
	// ���ڿ� �ڿ��� �׻� �ι��ڰ� ������ �ؼ� strlen���� ���� ���� �� �ֵ���
	uint16_t id = *(uint16_t*)_packet;					_packet += sizeof(uint16_t);
	uint32_t nicknameSize = wcslen((wchar_t*)_packet);
	wchar_t* nickname = new wchar_t[(nicknameSize * 2) + 2];
	memcpy(nickname, _packet, (nicknameSize * 2) + 2);

	User* pUser = UserManager::GetInst()->GetUser(id);
	if (pUser == nullptr)
	{
		printf("C_Enter ó�� ���� ����! : %d�� ã�� �� ���� id�Դϴ�.\n", id);
		return;
	}
	pUser->SetNickname(nickname);
	std::wcout << nickname << "�� �α��� �Ϸ�!" << '\n';
}
