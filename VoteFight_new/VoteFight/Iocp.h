#pragma once

class Socket;
class IocpEvents;

// I/O Completion Port
class Iocp {
public:
	HANDLE				 m_hIocp;				// IOCP 핸들

	// 1회의 GetQueuedCompletionStatus에서 최대로 꺼내올 수 있는 I/O 이벤트의 수
	static const int	m_MaxEventCount = 1000;	
	int					m_threadCount;			// IOCP에 할당된 스레드의 수

public:
	
	Iocp(int threadCount);						// 지정된 스레드 수로 IOCP 객체를 생성
	~Iocp();									// IOCP 객체를 정리하고 관련된 자원을 해제

	// 소켓을 IOCP에 추가, 소켓에 대한 이벤트를 받아오기 위해 사용자 지정 포인터를 함께 제공
	void				Add(Socket& _socket, void* _userPtr);	
	void				Wait(IocpEvents &_output, int _timeoutMs);	// IOCP에서 이벤트를 대기
};

// IOCP에서 수신된 이벤트 클래스
class IocpEvents {
public:
	// GetQueuedCompletionStatus에서 가져온 이벤트들을 저장하는 배열
	OVERLAPPED_ENTRY	m_events[Iocp::m_MaxEventCount];
	int					m_eventCount;			// 저장된 이벤트의 수
};


