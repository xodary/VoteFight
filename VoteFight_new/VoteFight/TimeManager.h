#pragma once

class CTimeManager : public CSingleton<CTimeManager>
{
	friend class CSingleton;

private:
	LARGE_INTEGER m_previousCount;
	LARGE_INTEGER m_currentCount;
	LARGE_INTEGER m_frequency;

	float         m_deltaTime; // ���� �����Ӱ� ���� ������ ���̿� �帥 �ð�
	float		  m_accTime;   // 1�ʸ� �����ϱ� ���� ���� �ð�

	UINT		  m_callCount; // �Լ� ȣ�� Ƚ��
	UINT		  m_fps;	   // �ʴ� �Լ� ȣ�� Ƚ��
	UINT		  m_fpsLimit;  // �����ӷ���Ʈ�� �Ѱ� ��

private:
	CTimeManager();
	virtual ~CTimeManager();

public:
	void Init();

	float GetDeltaTime();

	void Update();
};
