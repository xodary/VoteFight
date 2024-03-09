#pragma once
#include "Component.h"

class CSpriteRenderer : public CComponent
{
private:
	bool   m_isLoop;

	XMINT2 m_spriteSize;
	int    m_startFrameIndex;
	int	   m_endFrameIndex;
	int	   m_frameIndex;

	float  m_duration;
	float  m_elapsedTime;

public:
	CSpriteRenderer();
	virtual ~CSpriteRenderer();

	void SetSpriteSize(const XMINT2& spriteSize);
	const XMINT2& GetSpriteSize();

	void SetFrameIndex(int frameIndex);
	int GetFrameIndex();

	void SetDuration(float duration);
	float GetDuration();

	virtual void UpdateShaderVariables();

	void Play(bool isLoop, int startFrameIndex, int endFrameIndex, float duration);

	virtual void Update();
};
