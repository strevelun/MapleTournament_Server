#pragma once
class PacketHandler
{
public:
	PacketHandler();
	~PacketHandler();

public:
	void C_Enter(char* _packet); // 플레이어가 닉네임 입력하면 플레이어한테서 오는 패킷을 처리하는 함수
};

