#include "pch.h"
#include "NPC.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "StateMachine.h"
#include "Transform.h"
#include "NPCStates.h"

CNPC::CNPC() :
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("Player");
}

CNPC::~CNPC()
{
}


string CNPC::GetSpineName()
{
	return m_spineName;
}

float CNPC::GetSpineAngle()
{
	return m_spineAngle;
}

void CNPC::SetSpineAngle(float angle)
{
	m_spineAngle = angle;
}

void CNPC::Init()
{
	//CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

//	stateMachine->SetCurrentState(CNPCIdleState::GetInstance());

	//CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	//animator->SetWeight("idle", 1.0f);
}

void CNPC::Update()
{
	CObject::Update();

}

void CNPC::OnCollisionEnter(CObject* collidedObject)
{
	cout << collidedObject->GetName() << endl;
	collidedObject->SetDeleted(true);
}
