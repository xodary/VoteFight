#pragma once
#include <list>
#include <memory>

#include "../../Packet.h"

class PacketQueue
{
public:
	static std::list<char[MAX_BUFSIZE_CLIENT]> SendQueue;
	static std::list<char[MAX_BUFSIZE_CLIENT]> RecvQueue;

	static short SendQueueIndex;
	static short RecvQueueIndex;
public:
	static void AddSendPacket(void* packet);
	static void AddRecvPacket(void* packet);

	static void PopSendPacket();
	static void PopRecvPacket();
};

