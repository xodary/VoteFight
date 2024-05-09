#include "pch.h"
#include "Poll.h"

// fdArray와 fdArrayLength가 가리키는 배열의 소켓 중 하나 이상이 이벤트를 일으킬 때까지 기다립니다.
// timeOutMs는 최대 대기 시간을 밀리초 단위로 기다리는 값입니다.

// _fdArray        : PollFD 구조체 배열 - 감지할 소켓 정보
// _fdArrayLength  : PollFD 배열의 길이 - 배열에 포함된 소켓의 수
// _timeOutMs      : 이벤트를 감지할 때 사용할 시간 제한

int Poll(PollFD* _fdArray, int _fdArrayLength, int _timeOutMs)
{
	// 구조체가 예상된 형태로 정의되어 있는지를 검사
	static_assert(sizeof(_fdArray[0]) == sizeof(_fdArray[0].m_pollfd), ""); // 이걸 만족 못하면 PollFD에 virtual 함수 등 다른 멤버가 들어갔다는 뜻이다. 이러한 경우 PollFD로부터 네이티브 배열로의 복사를 해주는 것을 별도로 구현해야 한다.
#ifdef _WIN32
	return ::WSAPoll((WSAPOLLFD*)_fdArray, _fdArrayLength, _timeOutMs);
#else
	return ::poll((pollfd*)_fdArray, _fdArrayLength, _timeOutMs);
#endif
}
