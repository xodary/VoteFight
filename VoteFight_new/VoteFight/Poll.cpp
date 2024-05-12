#include "pch.h"
#include "Poll.h"

// fdArray�� fdArrayLength�� ����Ű�� �迭�� ���� �� �ϳ� �̻��� �̺�Ʈ�� ����ų ������ ��ٸ��ϴ�.
// timeOutMs�� �ִ� ��� �ð��� �и��� ������ ��ٸ��� ���Դϴ�.

// _fdArray        : PollFD ����ü �迭 - ������ ���� ����
// _fdArrayLength  : PollFD �迭�� ���� - �迭�� ���Ե� ������ ��
// _timeOutMs      : �̺�Ʈ�� ������ �� ����� �ð� ����

int Poll(PollFD* _fdArray, int _fdArrayLength, int _timeOutMs)
{
	// ����ü�� ����� ���·� ���ǵǾ� �ִ����� �˻�
	static_assert(sizeof(_fdArray[0]) == sizeof(_fdArray[0].m_pollfd), ""); // �̰� ���� ���ϸ� PollFD�� virtual �Լ� �� �ٸ� ����� ���ٴ� ���̴�. �̷��� ��� PollFD�κ��� ����Ƽ�� �迭���� ���縦 ���ִ� ���� ������ �����ؾ� �Ѵ�.
#ifdef _WIN32
	return ::WSAPoll((WSAPOLLFD*)_fdArray, _fdArrayLength, _timeOutMs);
#else
	return ::poll((pollfd*)_fdArray, _fdArrayLength, _timeOutMs);
#endif
}
