#include "pch.h"
#include "StateMachine.h"

#include "State.h"

CStateMachine::CStateMachine() :
	m_previousState(),
	m_currentState()
{
}

CStateMachine::~CStateMachine()
{
}

CState* CStateMachine::GetPreviousState()
{
	return m_previousState;
}

void CStateMachine::SetCurrentState(CState* state)
{
	m_currentState = state;

	if (m_currentState != nullptr)
	{
		m_currentState->Enter(m_owner);
	}
}

CState* CStateMachine::GetCurrentState()
{
	return m_currentState;
}

void CStateMachine::ChangeState(CState* state)
{
	if ((state != nullptr) && (m_currentState != state))
	{
		m_currentState->Exit(m_owner);
		m_previousState = m_currentState;
		m_currentState = state;
		m_currentState->Enter(m_owner);
	}
}

void CStateMachine::Update()
{
	if (m_isEnabled && m_currentState != nullptr)
	{
		m_currentState->Update(m_owner);
	}
}
