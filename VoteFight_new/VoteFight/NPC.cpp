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
#include "UI.h"

CNPC::CNPC() :
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("NPC");
	m_Quest = new CQuest("첫 번째 퀘스트", "첫 번째 퀘스트를 완료하세요!", C_NPC_Item("FireWood_01", 1));
	SetGroupType((UINT)GROUP_TYPE::NPC);
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
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	stateMachine->SetCurrentState(CNPCIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->SetWeight("idle", 1.0f);

	m_bilboardUI.push_back(new CSpeechBubbleUI(this));
}

void CNPC::Update()
{
	CCharacter::Update();
}

void CNPC::OnCollisionEnter(CObject* collidedObject)
{
	if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::PLAYER)
	{
		if (!m_Quest->getCompletionStatus())
		{
			cout << "안녕!";
			m_Quest->show();
		}
	}
}

