#pragma once 
#include <mutex>
#include <condition_variable>

// thread 간의 동기화
class Semaphore {
private:
	std::mutex					m_mtx;			// 스레드 간의 동기화를 위한 뮤텍스 객체
	std::condition_variable		m_cv;			// 대기 중인 스레드를 깨우기 위한 조건 변수
	int							m_count;		// 세마포어의 현재 상태를 나타내는 변수
public:
	Semaphore(int count_ = 0)
		: m_count(count_) {}

	inline void Notify()						// 세마포어에서 리소스를 가져오기 위해 대기하는 메서드
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_count++;
		m_cv.notify_one();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mtx);	// 세마포어에서 리소스를 가져오기 위해 대기하는 메서드

		while (m_count == 0) {
			m_cv.wait(lock);
		}
		m_count--;
	}
};