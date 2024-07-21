#pragma once
#include "ImaysNet/Socket.h"

class RemoteClient;

enum EVENT_TYPE { EV_UPDATE, EV_PHASE, EV_ANIMATION };

struct TIMER_EVENT {
	RemoteClient* obj_id;
	chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	UINT target_id;
	UINT grouptype;
	string aniamtion;
	UINT bone;

	constexpr bool operator < (const TIMER_EVENT& L) const { return (wakeup_time > L.wakeup_time); }
};

class CTimer
{
public:
	static concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
	static int phase;

	CTimer() {}
	~CTimer() {}

	static void do_timer();
};

