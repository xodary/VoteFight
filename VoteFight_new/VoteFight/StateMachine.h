#pragma once
#include "Component.h"

class CState;

class CStateMachine : public CComponent
{
private:
	CState* m_previousState;
	CState* m_currentState;

public:
	CStateMachine();
	virtual ~CStateMachine();

	CState* GetPreviousState();

	void SetCurrentState(CState* state);
	CState* GetCurrentState();

	void ChangeState(CState* state);

	virtual void Update();
};
