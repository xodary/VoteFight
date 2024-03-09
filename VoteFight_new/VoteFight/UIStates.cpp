#include "pch.h"
#include "UIStates.h"

#include "InputManager.h"

#include "UI.h"

#include "StateMachine.h"
#include "Animator.h"

CMissionUIShowState::CMissionUIShowState()
{
}

CMissionUIShowState::~CMissionUIShowState()
{
}

void CMissionUIShowState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->Play("Show", false, true);
}

void CMissionUIShowState::Exit(CObject* object)
{
}

void CMissionUIShowState::Update(CObject* object)
{
	if (KEY_TAP(KEY::TAB))
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(object->GetComponent(COMPONENT_TYPE::STATE_MACHINE));

		stateMachine->ChangeState(CMissionUIHideState::GetInstance());
	}
}

//=========================================================================================================================

CMissionUIHideState::CMissionUIHideState()
{
}

CMissionUIHideState::~CMissionUIHideState()
{
}

void CMissionUIHideState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->Play("Hide", false, true);
}

void CMissionUIHideState::Exit(CObject* object)
{
}

void CMissionUIHideState::Update(CObject* object)
{
	if (KEY_TAP(KEY::TAB))
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(object->GetComponent(COMPONENT_TYPE::STATE_MACHINE));

		stateMachine->ChangeState(CMissionUIShowState::GetInstance());
	}
}

//=========================================================================================================================

CKeyUIActivateState::CKeyUIActivateState()
{
}

CKeyUIActivateState::~CKeyUIActivateState()
{
}

void CKeyUIActivateState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->Play("Activate", false, true);
}

void CKeyUIActivateState::Exit(CObject* object)
{
}

void CKeyUIActivateState::Update(CObject* object)
{
}

//=========================================================================================================================

CHitUIFadeState::CHitUIFadeState()
{
}

CHitUIFadeState::~CHitUIFadeState()
{
}

void CHitUIFadeState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	object->SetActive(true);
	animator->Play("Fade", false, true);
}

void CHitUIFadeState::Exit(CObject* object)
{
}

void CHitUIFadeState::Update(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	if (animator->IsFinished())
	{
		object->SetActive(false);
	}
}
