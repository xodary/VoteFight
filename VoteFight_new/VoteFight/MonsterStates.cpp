#include "pch.h"
#include "MonsterStates.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Monster.h"
#include "StateMachine.h"
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"
#include "GameFramework.h"
#include <bitset>

CMonsterIdleState::CMonsterIdleState()
{
}


CMonsterIdleState::~CMonsterIdleState()
{
}

void CMonsterIdleState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("idle", true);
}

void CMonsterIdleState::Exit(CObject* object)
{
}

void CMonsterIdleState::Update(CObject* object)
{
	CMonster* monster = static_cast<CMonster*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(monster->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(monster->GetComponent(COMPONENT_TYPE::TRANSFORM));

}
