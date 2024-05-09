#include "pch.h"
#include "PacketQueue.h"

// �۽� ��Ŷ�� �����ϴ� ť
std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::m_SendQueue;

// ���� ��Ŷ�� �����ϴ� ť
std::list<char[MAX_BUFSIZE_CLIENT]> PacketQueue::m_RecvQueue;

short PacketQueue::m_SendQueueIndex = 0;  // �۽� ��Ŷ�� �ε���
short PacketQueue::m_RecvQueueIndex = 0;  // ���� ��Ŷ�� �ε���

void PacketQueue::AddSendPacket(void* _packet)	// �۽� ��Ŷ�� ť�� �߰�
{
	char* p = reinterpret_cast<char*>(_packet);

	// Send ť�� ��������� ���ο� ���۸� ����
	if (m_SendQueue.empty())
		m_SendQueue.emplace_back();

	// Send ���ۿ� ��Ŷ�� �߰�
	// ���� ���ۿ� ��Ŷ�� �߰��� �� �ִ� ���
	if (m_SendQueueIndex + p[0] < MAX_BUFSIZE_CLIENT) {
		memcpy(m_SendQueue.back() + m_SendQueueIndex, p, p[0]);
		m_SendQueueIndex += p[0];
	}
	else { // ���� ���ۿ� �߰��� �� ���� ��� : ���ο� ���� ���� & ��Ŷ �߰�
		m_SendQueueIndex = 0;
		m_SendQueue.emplace_back();
		memcpy(m_SendQueue.back() + m_SendQueueIndex, p, p[0]);
	}
}

void PacketQueue::AddRecvPacket(void* _packet)
{

}

// �۽� ��Ŷ�� ť���� ����
void PacketQueue::PopSendPacket()
{
	m_SendQueueIndex = 0;
	m_SendQueue.pop_front();
}

// ���� ��Ŷ�� ť���� ����
void PacketQueue::PopRecvPacket()
{
	m_RecvQueueIndex = 0;
	m_RecvQueue.pop_front();
}
