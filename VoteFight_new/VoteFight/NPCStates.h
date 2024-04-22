#pragma once
#include "State.h"

class CNPCIdleState : public CState, public CSingleton<CNPCIdleState>
{
	friend class CSingleton<CNPCIdleState>;

private:
	CNPCIdleState();
	virtual ~CNPCIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};
