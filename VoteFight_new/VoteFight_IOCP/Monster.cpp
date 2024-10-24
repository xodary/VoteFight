#include "pch.h"
#include "Monster.h"
#include "StateMachine.h"
#include "RemoteClient.h"
#include "State.h"

CMonster::CMonster() : CObject()
{
	m_meet = 1;
	m_upAnimation = "idle";
	m_stateMachine = new CStateMachine();
	m_stateMachine->m_owner = this;
	m_stateMachine->SetCurrentState(CMonsterGoUpState::GetInstance());
}

CMonster::~CMonster()
{
}