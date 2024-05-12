#include "pch.h"
#include "PacketQueue.h"

// 송신 패킷을 저장하는 큐
std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::m_SendQueue;

// 수신 패킷을 저장하는 큐
std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::m_RecvQueue;

short PacketQueue::m_SendQueueIndex = 0;  // 송신 패킷의 인덱스
short PacketQueue::m_RecvQueueIndex = 0;  // 수신 패킷의 인덱스

void PacketQueue::AddSendPacket(void* _packet)	// 송신 패킷을 큐에 추가
{
	char* p = reinterpret_cast<char*>(_packet);

	// Send 큐가 비어있으면 새로운 버퍼를 생성
	if (m_SendQueue.empty())
		m_SendQueue.emplace_back();

	// Send 버퍼에 패킷을 추가
	// 현재 버퍼에 패킷을 추가할 수 있는 경우
	if (m_SendQueueIndex + p[0] < MAX_BUFSIZE_CLIENT) {
		memcpy(m_SendQueue.back() + m_SendQueueIndex, p, p[0]);
		m_SendQueueIndex += p[0];
	}
	else { // 현재 버퍼에 추가할 수 없는 경우 : 새로운 버퍼 생성 & 패킷 추가
		m_SendQueueIndex = 0;
		m_SendQueue.emplace_back();
		memcpy(m_SendQueue.back() + m_SendQueueIndex, p, p[0]);
	}
}

void PacketQueue::AddRecvPacket(void* _packet)
{

}

// 송신 패킷을 큐에서 제거
void PacketQueue::PopSendPacket()
{
	m_SendQueueIndex = 0;
	m_SendQueue.pop_front();
}

// 수신 패킷을 큐에서 제거
void PacketQueue::PopRecvPacket()
{
	m_RecvQueueIndex = 0;
	m_RecvQueue.pop_front();
}
