#pragma once

class CState;

class CStateMachine
{
private:
	CState* m_previousState;
	CState* m_currentState;

public:
	CStateMachine();
	virtual ~CStateMachine();

	CState* GetPreviousState();

	void SetCurrentState(CState* state);
	void SetCurrentState(STATE_ENUM n);
	CState* GetCurrentState();

	void ChangeState(CState* state);

	virtual void Update();
};
