#include "pch.h"
#include "ImaysNet/Socket.h"
#include "ImaysNet/iocp.h"
#include "Timer.h"

//const int					numWorkerTHREAD{ 1 };	// Worker Thread Count
Iocp Iocp::iocp;
concurrency::concurrent_priority_queue<TIMER_EVENT> CTimer::timer_queue;

void CTimer::do_timer()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == CTimer::timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				CTimer::timer_queue.push(ev);		// ����ȭ �ʿ�
				// timer_queue�� �ٽ� ���� �ʰ� ó���ؾ� �Ѵ�.
				this_thread::sleep_for(10ms);  // ����ð��� ���� �ȵǾ����Ƿ� ��� ���
				continue;
			}
			switch (ev.event_id) {
			case EV_UPDATE:
			{
				EXP_OVER* ov = new EXP_OVER;
				ov->m_ioType = IO_TYPE::IO_UPDATE;
				PostQueuedCompletionStatus(Iocp::iocp.m_hIocp, 1, (ULONG_PTR)ev.obj_id, &ov->m_wsa_over);
			}
			break;
			case EV_PHASE:
			{
				EXP_OVER* ov = new EXP_OVER;
				ov->m_ioType = IO_TYPE::IO_PHASE;
				PostQueuedCompletionStatus(Iocp::iocp.m_hIocp, 1, (ULONG_PTR)ev.obj_id, &ov->m_wsa_over);
			}
			break;
			}
			continue;		// ��� ���� �۾� ������
		}
		this_thread::sleep_for(1ms);   // timer_queue�� ��� ������ ��� ��ٷȴٰ� �ٽ� ����
	}
}