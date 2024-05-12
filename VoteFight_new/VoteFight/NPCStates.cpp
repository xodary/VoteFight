#include "pch.h"
#include "NPCStates.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "NPC.h"
#include "StateMachine.h"
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"
#include "GameFramework.h"
#include <bitset>

CNPCIdleState::CNPCIdleState()
{
}


CNPCIdleState::~CNPCIdleState()
{
}

void CNPCIdleState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("idle", true);
}

void CNPCIdleState::Exit(CObject* object)
{
}

void CNPCIdleState::Update(CObject* object)
{
	CNPC* npc = static_cast<CNPC*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(npc->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(npc->GetComponent(COMPONENT_TYPE::TRANSFORM));

}
