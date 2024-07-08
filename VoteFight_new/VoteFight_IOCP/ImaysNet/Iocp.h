#pragma once

class Socket;
class IocpEvents;
class EXP_OVER;

// I/O Completion Port 객체.
class Iocp {
public:
	static Iocp iocp;
	static const int MaxEventCount = 1000;
	
	Iocp();
	~Iocp();

	void Add(Socket& socket, void* userPtr);
	void Add(EXP_OVER* exover, int npc_id);
	
	HANDLE m_hIocp;
	void Wait(IocpEvents &output, int timeoutMs);
};

// IOCP의 GetQueuedCompletionStatus로 받은 I/O 완료신호들
class IocpEvents
{
public:
	// GetQueuedCompletionStatus으로 꺼내온 이벤트들
	OVERLAPPED_ENTRY m_events[Iocp::MaxEventCount];
	int m_eventCount;
};
