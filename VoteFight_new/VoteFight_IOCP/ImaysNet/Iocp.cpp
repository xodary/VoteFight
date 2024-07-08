#include "../pch.h"
#include "Iocp.h"
#include "Socket.h"
#include "Exception.h"

Iocp::Iocp()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

Iocp::~Iocp()
{
	CloseHandle(m_hIocp);
}

// IOCP에 socket을 추가합니다.
void Iocp::Add(Socket& socket, void* userPtr)
{
	if (!CreateIoCompletionPort((HANDLE)socket.m_fd, m_hIocp, (ULONG_PTR)userPtr, 0))
		throw Exception("IOCP add failed!");
}

// IOCP에 Event를 추가합니다.
void Iocp::Add(EXP_OVER* exover, int id)
{
	if (!PostQueuedCompletionStatus(m_hIocp, 1, id, &exover->m_wsa_over))
		throw Exception("IOCP add Event failed!");
}


void Iocp::Wait(IocpEvents &output, int timeoutMs)
{
	 BOOL r = GetQueuedCompletionStatusEx(m_hIocp, (LPOVERLAPPED_ENTRY)output.m_events, MaxEventCount, (ULONG*)&output.m_eventCount, timeoutMs, FALSE);
	 if (!r)
	 {
		 output.m_eventCount = 0;
	 }
}
