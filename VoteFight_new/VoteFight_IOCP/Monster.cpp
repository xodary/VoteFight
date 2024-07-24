#include "pch.h"
#include "Monster.h"
#include "StateMachine.h"
#include "RemoteClient.h"
#include "State.h"

CMonster::CMonster() : m_meet(1)
{
	m_upAnimation = "idle";
	m_stateMachine = new CStateMachine();
	m_stateMachine->m_owner = this;
	m_stateMachine->SetCurrentState(CMonsterIdleState::GetInstance());
}

CMonster::~CMonster()
{
}