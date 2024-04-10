#pragma once
#include "State.h"

class CMissionUIShowState : public CState, public CSingleton<CMissionUIShowState>
{
	friend class CSingleton<CMissionUIShowState>;

private:
	CMissionUIShowState();
	virtual ~CMissionUIShowState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};