#pragma once

class CState;
class CObject;

class CStateMachine
{
public:
	CState* m_previousState;
	CState* m_currentState;

	CObject* m_owner;

	CStateMachine();
	virtual ~CStateMachine();

	CState* GetPreviousState();

	void SetCurrentState(CState* state);
	CState* GetCurrentState();

	void ChangeState(CState* state);

	virtual void Update();
};
