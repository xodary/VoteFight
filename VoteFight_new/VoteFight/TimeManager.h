#pragma once

class CTimeManager : public CSingleton<CTimeManager>
{
	friend class CSingleton<CTimeManager>;

private:
	LARGE_INTEGER m_previousCount;
	LARGE_INTEGER m_currentCount;
	LARGE_INTEGER m_frequency;

	float         m_deltaTime; // 이전 프레임과 현재 프레임 사이에 흐른 시간
	float		  m_accTime;   // 1초를 측정하기 위한 누적 시간

	UINT		  m_callCount; // 함수 호출 횟수
	UINT		  m_fps;	   // 초당 함수 호출 횟수
	UINT		  m_fpsLimit;  // 프레임레이트의 한계 값

private:
	CTimeManager();
	virtual ~CTimeManager();

public:
	void Init();

	float GetDeltaTime();

	void Update();
};
