#pragma once
#include "State.h"

class CPlayerIdleState : public CState, public CSingleton<CPlayerIdleState>
{
	friend class CSingleton;

private:
	CPlayerIdleState();
	virtual ~CPlayerIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};