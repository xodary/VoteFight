#pragma once 

#ifndef _WIN32
#include <poll.h>
#endif

// unix의 poll, windows의 WSAPoll을 위한 구조체입니다.
// 다중 소켓 이벤트를 감지 클래스
class PollFD {
public:
#ifdef _WIN32
	WSAPOLLFD	m_pollfd;		//  다중 소켓 이벤트를 감지하는 데 사용되는 구조체
#else
	pollfd m_pollfd;
#endif
};

// 다중 소켓 이벤트를 감지 함수
int Poll(PollFD* _fdArray, int _fdArrayLength, int _timeOutMs);