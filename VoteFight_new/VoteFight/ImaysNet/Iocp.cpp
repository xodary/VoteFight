#include "../pch.h"
#include "Iocp.h"
#include "Socket.h"
#include "Exception.h"

Iocp::Iocp(int threadCount)
{
	// IOCP 핸들을 생성하고 지정된 스레드 수로 초기화
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
	m_threadCount = threadCount;	// 사용된 스레드 수를 저장
}

Iocp::~Iocp()
{
	CloseHandle(m_hIocp);			// IOCP 핸들 닫기
}

// IOCP에 socket을 추가합니다.
void Iocp::Add(Socket& _socket, void* _userPtr)
{
	// 소켓을 IOCP에 추가 & 성공하지 않으면 예외
	if (!CreateIoCompletionPort((HANDLE)_socket.m_fd, m_hIocp, (ULONG_PTR)_userPtr, m_threadCount))
		throw Exception(" [ Exception ] IOCP add failed!");
}

void Iocp::Wait(IocpEvents &_output, int _timeoutMs)
{
	// GetQueuedCompletionStatusEx 함수를 사용하여 IOCP에서 이벤트를 가져옴
	 BOOL r = GetQueuedCompletionStatusEx(m_hIocp, (LPOVERLAPPED_ENTRY)_output.m_events, m_MaxEventCount, (ULONG*)&_output.m_eventCount, _timeoutMs, FALSE);
	 if (!r) // 대기 도중 에러가 발생하면 이벤트 카운트를 0으로 설정
		 _output.m_eventCount = 0;	 
}
