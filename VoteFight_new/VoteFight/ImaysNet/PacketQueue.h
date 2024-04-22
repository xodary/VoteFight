#pragma once
#include <list>
#include <memory>

#include "../../Packet.h"

class PacketQueue
{
public:
	static std::list<char[MAX_BUFSIZE_CLIENT]>	m_SendQueue;
	static std::list<char[MAX_BUFSIZE_CLIENT]>	m_RecvQueue;

	static short								m_SendQueueIndex;
	static short								m_RecvQueueIndex;
public:
	static void									AddSendPacket(void* _packet);
	static void									AddRecvPacket(void* _packet);

	static void									PopSendPacket();
	static void									PopRecvPacket();
};

