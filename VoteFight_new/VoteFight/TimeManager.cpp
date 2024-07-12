#include "pch.h"
#include "TimeManager.h"
#include "GameFramework.h"

CTimeManager::CTimeManager() :
	m_previousCount(),
	m_currentCount(),
	m_frequency(),
	m_deltaTime(),
	m_accTime(),
	m_callCount(),
	m_fps(),
	m_fpsLimit()
{
}

CTimeManager::~CTimeManager()
{
}

void CTimeManager::Init()
{
	// 초당 카운트 횟수
	QueryPerformanceFrequency(&m_frequency);

	// 이전 카운트
	QueryPerformanceCounter(&m_previousCount);
}

float CTimeManager::GetDeltaTime()
{
	return m_deltaTime;
}

void CTimeManager::Update()
{
	QueryPerformanceCounter(&m_currentCount);

	// 현재 프레임의 카운트와 이전 프레임의 카운트 차이를 초당 카운트 횟수로 나누어 초당 카운트의 차를 구한다.
	float deltaTime = (float)(m_currentCount.QuadPart - m_previousCount.QuadPart) / m_frequency.QuadPart;

	// 프레임 레이트의 한계 값이 존재할 경우, 프레임 레이트를 강제로 제한한다.
	if (m_fpsLimit > 0)
	{
		while (deltaTime < 1.0f / m_fpsLimit)
		{
			QueryPerformanceCounter(&m_currentCount);
			deltaTime = (float)(m_currentCount.QuadPart - m_previousCount.QuadPart) / m_frequency.QuadPart;
		}
	}

	m_deltaTime = deltaTime;

	// 다음 번 계산을 위해여 이전 카운트의 값을 현재 카운트 값으로 갱신한다.
	m_previousCount = m_currentCount;

	// 호출 횟수를 1 증가시킨다.
	++m_callCount;

	// 흐른 시간을 누적한다.
	m_accTime += m_deltaTime;

	if (m_accTime >= 1.0f)
	{
		m_fps = m_callCount;
		m_callCount = 0;
		m_accTime = 0.0f;

		char buffer[32] = {};

		sprintf_s(buffer, sizeof(buffer), "FPS: %d", m_fps);
		SetWindowTextA(CGameFramework::GetInstance()->GetHwnd(), buffer);
	}

	m_lastTime -= m_deltaTime;
	cout << "Last Time : " << m_lastTime << endl;
}
