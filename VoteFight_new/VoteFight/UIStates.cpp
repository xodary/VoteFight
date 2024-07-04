#include "pch.h"
#include "UIStates.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "Scene.h"
#include "UI.h"

#include "StateMachine.h"
#include "Animator.h"

CInventoryUIState::CInventoryUIState()
{
}

CInventoryUIState::~CInventoryUIState()
{
}

void CInventoryUIState::Enter(CObject* object)
{
	vector<CObject*> uis = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::UI);
	for (auto& ui : uis) {
		delete ui;
	}
	uis.clear();


}

void CInventoryUIState::Exit(CObject* object)
{
}

void CInventoryUIState::Update(CObject* object)
{
	if (KEY_TAP(KEY::TAB))
	{



		CStateMachine* stateMachine = static_cast<CStateMachine*>(object->GetComponent(COMPONENT_TYPE::STATE_MACHINE));

		stateMachine->ChangeState(CInventoryUIState::GetInstance());
	}
}
