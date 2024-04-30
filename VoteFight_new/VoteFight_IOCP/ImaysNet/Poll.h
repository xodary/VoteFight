#pragma once 

#ifndef _WIN32
#include <poll.h>
#endif

// unix�� poll, windows�� WSAPoll�� ���� ����ü�Դϴ�.
class PollFD
{
public:
#ifdef _WIN32
	WSAPOLLFD m_pollfd;
#else
	pollfd m_pollfd;
#endif
};

int Poll(PollFD* fdArray, int fdArrayLength, int timeOutMs);