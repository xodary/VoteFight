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
	// �ʴ� ī��Ʈ Ƚ��
	QueryPerformanceFrequency(&m_frequency);

	// ���� ī��Ʈ
	QueryPerformanceCounter(&m_previousCount);
}

float CTimeManager::GetDeltaTime()
{
	return m_deltaTime;
}

void CTimeManager::Update()
{
	QueryPerformanceCounter(&m_currentCount);

	// ���� �������� ī��Ʈ�� ���� �������� ī��Ʈ ���̸� �ʴ� ī��Ʈ Ƚ���� ������ �ʴ� ī��Ʈ�� ���� ���Ѵ�.
	float deltaTime = (float)(m_currentCount.QuadPart - m_previousCount.QuadPart) / m_frequency.QuadPart;

	// ������ ����Ʈ�� �Ѱ� ���� ������ ���, ������ ����Ʈ�� ������ �����Ѵ�.
	if (m_fpsLimit > 0)
	{
		while (deltaTime < 1.0f / m_fpsLimit)
		{
			QueryPerformanceCounter(&m_currentCount);
			deltaTime = (float)(m_currentCount.QuadPart - m_previousCount.QuadPart) / m_frequency.QuadPart;
		}
	}

	m_deltaTime = deltaTime;

	// ���� �� ����� ���ؿ� ���� ī��Ʈ�� ���� ���� ī��Ʈ ������ �����Ѵ�.
	m_previousCount = m_currentCount;

	// ȣ�� Ƚ���� 1 ������Ų��.
	++m_callCount;

	// �帥 �ð��� �����Ѵ�.
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
