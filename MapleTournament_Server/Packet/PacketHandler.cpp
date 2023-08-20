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
	// ���ڿ� �ڿ��� �׻� �ι��ڰ� ������ �ؼ� strlen���� ���� ���� �� �ֵ���
	uint16_t id = *(uint16_t*)_packet;					_packet += sizeof(uint16_t);
	uint32_t nicknameSize = wcslen((wchar_t*)_packet);
	wchar_t* nickname = new wchar_t[(nicknameSize * 2) + 2];
	memcpy(nickname, _packet, (nicknameSize * 2) + 2);
	std::wstring strNickname = nickname;

	User* pUser = UserManager::GetInst()->GetUser(id);
	if (pUser == nullptr)
	{
		printf("C_Enter ó�� ���� ����! : %d�� ã�� �� ���� id�Դϴ�.\n", id);
		return;
	}

	User* pOtherUser = UserManager::GetInst()->FindUserByNickname(pUser->GetNickname());
	if(!pOtherUser) 
		pUser->SetNickname(strNickname);
	
	if (!pOtherUser || pOtherUser->GetId() == pUser->GetId())
	{
		OKLoginPacket p;
		send((SOCKET)pUser->GetId(), p.GetPacketBuffer(), p.GetPacketSize(), 0);
		std::wcout << strNickname << "�� �α��� �Ϸ�!" << '\n';
	}
	else
	{
		FailedLoginPacket p;
		send((SOCKET)pUser->GetId(), p.GetPacketBuffer(), p.GetPacketSize(), 0);
	}
}
