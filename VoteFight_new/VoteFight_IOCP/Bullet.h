#pragma once
#include "Object.h"

class CBullet : public CObject
{
public:
	chrono::system_clock::time_point		m_lastTime;
};

