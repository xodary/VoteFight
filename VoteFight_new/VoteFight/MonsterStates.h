#pragma once
#include "State.h"

class CMonsterIdleState : public CState, public CSingleton<CMonsterIdleState>
{
	friend class CSingleton<CMonsterIdleState>;

private:
	CMonsterIdleState();
	virtual ~CMonsterIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};
