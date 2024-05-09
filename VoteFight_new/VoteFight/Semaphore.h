#pragma once 
#include <mutex>
#include <condition_variable>

// thread ���� ����ȭ
class Semaphore {
private:
	std::mutex					m_mtx;			// ������ ���� ����ȭ�� ���� ���ؽ� ��ü
	std::condition_variable		m_cv;			// ��� ���� �����带 ����� ���� ���� ����
	int							m_count;		// ���������� ���� ���¸� ��Ÿ���� ����
public:
	Semaphore(int count_ = 0)
		: m_count(count_) {}

	inline void Notify()						// ��������� ���ҽ��� �������� ���� ����ϴ� �޼���
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_count++;
		m_cv.notify_one();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mtx);	// ��������� ���ҽ��� �������� ���� ����ϴ� �޼���

		while (m_count == 0) {
			m_cv.wait(lock);
		}
		m_count--;
	}
};