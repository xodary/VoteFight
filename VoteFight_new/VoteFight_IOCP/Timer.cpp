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
				CTimer::timer_queue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(10ms);  // 실행시간이 아직 안되었으므로 잠시 대기
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
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}