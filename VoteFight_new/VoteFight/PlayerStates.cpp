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
#include "GameFramework.h"
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
}

void CPlayerIdleState::Exit(CObject* object)
{
}

void CPlayerIdleState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
	// cout << transform->GetRotation().y << endl;
	if (KEY_TAP(KEY::W) || KEY_TAP(KEY::S) || KEY_TAP(KEY::A) || KEY_TAP(KEY::D))
	{
		float look(0.f);
		if (KEY_TAP(KEY::S))
			look = 180.f;
		if (KEY_TAP(KEY::A))
			look = 270.f;
		if (KEY_TAP(KEY::D))
			look = 90.f;
		player->SetTurnAngle(look);
		// transform->SetRotation(look);
		if(look - transform->GetRotation().y >= 90.f && look - transform->GetRotation().y < 270.f)
			stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		else
			stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
		// stateMachine->ChangeState(CPlayerWalkState::GetInstance());
	}

	if (KEY_HOLD(KEY::RBUTTON))
	{
		RECT rect;
		GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
		XMFLOAT2 cursor = CURSOR;

		float centerX = (rect.right - rect.left) / 2;
		float centerY = rect.bottom - rect.top / 2;
		
		float x = cursor.x - centerX;
		float y = centerY - cursor.y;

		float angle = XMConvertToDegrees(atan2(x, y));
		float look = transform->GetRotation().y;
		if (look > 180) look -= 360;
		if (-45 > angle - look)
		{
			stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
		}
		else if (45 < angle - look)
		{
			stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		}
		player->SetSpineAngle(angle);
	}
}

CPlayerWalkState::CPlayerWalkState()
{
}

CPlayerWalkState::~CPlayerWalkState()
{
}

void CPlayerWalkState::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("lisaWalk", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
}

void CPlayerWalkState::Exit(CObject* object)
{
}

void CPlayerWalkState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));

	if ((KEY_NONE(KEY::W) && KEY_NONE(KEY::S) && KEY_NONE(KEY::A) && KEY_NONE(KEY::D)) && Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	if (KEY_HOLD(KEY::SHIFT) && (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D)))
	{
		stateMachine->ChangeState(CPlayerRunState::GetInstance());
		return;
	}

	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D))
	{
		rigidBody->AddForce(Vector3::ScalarProduct(transform->GetForward(), 10000.0f * DT));
	}

	if (Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	CAnimator* animator = static_cast<CAnimator*>(player->GetComponent(COMPONENT_TYPE::ANIMATOR));
	XMFLOAT3 direction = Vector3::Normalize(rigidBody->GetVelocity());

}

CPlayerRunState::CPlayerRunState()
{
}

CPlayerRunState::~CPlayerRunState()
{
}

void CPlayerRunState::Enter(CObject* object)
{
}

void CPlayerRunState::Exit(CObject* object)
{
}

void CPlayerRunState::Update(CObject* object)
{
}

CPlayerLeftTurn::CPlayerLeftTurn()
	: restAngle(90.f)
{
}

CPlayerLeftTurn::~CPlayerLeftTurn()
{
}

void CPlayerLeftTurn::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);

	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = 90.f;
}

void CPlayerLeftTurn::Exit(CObject* object)
{
}

void CPlayerLeftTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 90.f * DT;
	if (restAngle < 0.f)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
	}
	else {
		float angle = transform->GetRotation().y - 90.f * DT;
		cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}
}

CPlayerRightTurn::CPlayerRightTurn()
	: restAngle(90.f)
{
}

CPlayerRightTurn::~CPlayerRightTurn()
{
}

void CPlayerRightTurn::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnRight", false);
	restAngle = 90.f;
}

void CPlayerRightTurn::Exit(CObject* object)
{
}

void CPlayerRightTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 90.f * DT;
	if (restAngle < 0.f)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
	}
	else {
		float angle = transform->GetRotation().y + 90.f * DT;
		cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}
}
