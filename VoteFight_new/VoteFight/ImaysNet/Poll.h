#pragma once 

#ifndef _WIN32
#include <poll.h>
#endif

// unix�� poll, windows�� WSAPoll�� ���� ����ü�Դϴ�.
// ���� ���� �̺�Ʈ�� ���� Ŭ����
class PollFD {
public:
#ifdef _WIN32
	WSAPOLLFD	m_pollfd;		//  ���� ���� �̺�Ʈ�� �����ϴ� �� ���Ǵ� ����ü
#else
	pollfd m_pollfd;
#endif
};

// ���� ���� �̺�Ʈ�� ���� �Լ�
int Poll(PollFD* _fdArray, int _fdArrayLength, int _timeOutMs);