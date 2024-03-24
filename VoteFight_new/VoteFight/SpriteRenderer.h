#pragma once
#include "Component.h"

class CSpriteRenderer : public CComponent
{
private:
	bool   m_isLoop;

	XMFLOAT2 m_spriteSize;
	int    m_startFrameIndex;
	int	   m_endFrameIndex;
	XMFLOAT2 m_frameLocation;

	float  m_duration;
	float  m_elapsedTime;

public:
	CSpriteRenderer();
	virtual ~CSpriteRenderer();

	void SetSpriteSize(const XMFLOAT2& spriteSize);
	const XMFLOAT2& GetSpriteSize();

	void SetFrameIndex(const XMFLOAT2& frameIndex);
	const XMFLOAT2& GetFrameIndex();

	void SetDuration(float duration);
	float GetDuration();

	virtual void UpdateShaderVariables();

	void Play(bool isLoop, int startFrameIndex, int endFrameIndex, float duration);

	virtual void Update();
};
