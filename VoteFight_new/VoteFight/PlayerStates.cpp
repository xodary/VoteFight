#include "pch.h"
#include "PlayerStates.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Player.h"
#include "StateMachine.h"
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"

#include "Camera.h"

CPlayerIdleState::CPlayerIdleState()
{
}

CPlayerIdleState::~CPlayerIdleState()
{
}

void CPlayerIdleState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("idle", true);
	animator->Play("lisaWalk", true);
	animator->SetWeight("lisaWalk", 0.2f);
	animator->SetWeight("idle", 0.8f);

}

void CPlayerIdleState::Exit(CObject* object)
{
}

void CPlayerIdleState::Update(CObject* object)
{
	if (KEY_HOLD(KEY::LBUTTON))
	{
		Sleep(1);
	}
}