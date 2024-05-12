#pragma once
#include "define.h"

class CObject;

class CState
{
protected:
	CState();
	virtual ~CState();

	STATE_ENUM stateNum = STATE_ENUM::CPlayerIdelState;

public:
	virtual void Enter(CObject* object) = 0;
	virtual void Exit(CObject* object) = 0;

	virtual void Update(CObject* object) = 0;

	STATE_ENUM GetStateNum() { return stateNum; }
};
