#include "pch.h"
#include "StateMachine.h"

#include "State.h"
#include "PlayerStates.h"

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

void CStateMachine::SetCurrentState(STATE_ENUM n)
{
	CState* state = nullptr;
	switch(n) {   //COtherPlayerIdleState
	case STATE_ENUM::CPlayerIdleState:
		state = CPlayerIdleState::GetInstance();
		break;
	case STATE_ENUM::CPlayerWalkState:
		state = CPlayerWalkState::GetInstance();
		break;
	}
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