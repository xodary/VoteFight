#include "../pch.h"
#include "PacketQueue.h"

std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::SendQueue;
std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::RecvQueue;

short PacketQueue::SendQueueIndex = 0;
short PacketQueue::RecvQueueIndex = 0;

void PacketQueue::AddSendPacket(void* packet)
{
	char* p = reinterpret_cast<char*>(packet);

	if (SendQueue.empty())
		SendQueue.emplace_back();

	//�̹� �ִ� Send ���۰� �߰��Ϸ��� ��Ŷ�� ���� �� �ִٸ�
	if (SendQueueIndex + p[0] < MAX_BUFSIZE_CLIENT) {
		memcpy(SendQueue.back() + SendQueueIndex, p, p[0]);
		SendQueueIndex += p[0];
	}
	else {
		SendQueueIndex = 0;
		SendQueue.emplace_back();
		memcpy(SendQueue.back() + SendQueueIndex, p, p[0]);
	}
}

void PacketQueue::AddRecvPacket(void* packet)
{

}

void PacketQueue::PopSendPacket()
{
	SendQueueIndex = 0;
	SendQueue.pop_front();
}

void PacketQueue::PopRecvPacket()
{
	RecvQueueIndex = 0;
	RecvQueue.pop_front();
}
