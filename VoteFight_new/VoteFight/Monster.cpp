#include "pch.h"
#include "Monster.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "StateMachine.h"
#include "Transform.h"
#include "MonsterStates.h"

CMonster::CMonster() :
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("Monster");
}

CMonster::~CMonster()
{
}


string CMonster::GetSpineName()
{
	return m_spineName;
}

float CMonster::GetSpineAngle()
{
	return m_spineAngle;
}

void CMonster::SetSpineAngle(float angle)
{
	m_spineAngle = angle;
}

void CMonster::Init()
{
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CMonsterIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->SetWeight("idle", 1.0f);
}

void CMonster::Update()
{
	CObject::Update();

}

