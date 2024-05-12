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
#include <bitset>

#include "./ImaysNet/ImaysNet.h"
#include "./ImaysNet/PacketQueue.h"

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

BOOL wasd = true;

void CPlayerIdleState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
	// cout << transform->GetRotation().y << endl;

	/*
	if (KEY_TAP(KEY::W) || KEY_TAP(KEY::S) || KEY_TAP(KEY::A) || KEY_TAP(KEY::D))
	{
		if (KEY_TAP(KEY::W))
			player->b_front = true;
		if (KEY_TAP(KEY::A))
			player->b_left = true;
		if (KEY_TAP(KEY::S))
			player->b_back = true;
		if (KEY_TAP(KEY::D))
			player->b_right = true;
		return;
	}
	*/

	if (KEY_NONE(KEY::W) || KEY_NONE(KEY::S) || KEY_NONE(KEY::A) || KEY_NONE(KEY::D))
	{
		if (KEY_NONE(KEY::W))
			player->b_front = false;
		if (KEY_NONE(KEY::A))
			player->b_left = false;
		if (KEY_NONE(KEY::S))
			player->b_back = false;
		if (KEY_NONE(KEY::D))
			player->b_right = false;
	}

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D))
	{
		if ((KEY_HOLD(KEY::W) && KEY_HOLD(KEY::S)) || (KEY_HOLD(KEY::A) && KEY_HOLD(KEY::D)))
			return;
		if (KEY_HOLD(KEY::W))
			player->b_front = true;
		if (KEY_HOLD(KEY::A))
			player->b_left = true;
		if (KEY_HOLD(KEY::S))
			player->b_back = true;
		if (KEY_HOLD(KEY::D))
			player->b_right = true;

		if (KEY_NONE(KEY::W))
			player->b_front = false;
		if (KEY_NONE(KEY::A))
			player->b_left = false;
		if (KEY_NONE(KEY::S))
			player->b_back = false;
		if (KEY_NONE(KEY::D))
			player->b_right = false;

		float wannaLook(0.f);
		
		if (player->b_front && player->b_right)
			wannaLook = 45.f;
		else if (player->b_front && player->b_left)
			wannaLook = 315.f;
		else if (player->b_back && player->b_right)
			wannaLook = 135.f;
		else if (player->b_back && player->b_left)
			wannaLook = 225.f;
		 else if (player->b_front)
			wannaLook = 0.f;
		else if (player->b_left)
			wannaLook = 270.f;
		else if (player->b_back)
			wannaLook = 180.f;
		else if (player->b_right)
			wannaLook = 90.f;
		else
			return;
		wannaLook -= 45.0f;
		player->SetTurnAngle(wannaLook );
		float nowRotation = wannaLook - transform->GetRotation().y;
		cout << nowRotation << endl;
		if ((0 < nowRotation && nowRotation <= 180) || nowRotation < -180)
			stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		else if (nowRotation > 180 || nowRotation < 0)
			stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
		else
			stateMachine->ChangeState(CPlayerWalkState::GetInstance());
		wasd = true;
	}
	else
		wasd = false;

	if (KEY_HOLD(KEY::RBUTTON) || KEY_TAP(KEY::RBUTTON))
	{
		RECT rect;
		GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
		XMFLOAT2 cursor = CURSOR;

		float centerX = (rect.right - rect.left) / 2;
		float centerY = (rect.bottom - rect.top) / 2;
		
		float x = cursor.x - centerX;
		float y = centerY - cursor.y;

		float angle = XMConvertToDegrees(atan2(x, y));
		player->SetClickAngle(angle);
		float look = transform->GetRotation().y;
		if (look > 180) look -= 360;
		float diff = angle - look;
		if (diff > 180) diff -= 360;
		if (diff < -180) diff += 360;
		if (-45 > diff)
		{
			if(-90 - 45 > diff)
				player->SetTurnAngle(look - 180.f);
			else
				player->SetTurnAngle(look - 90.f);
			stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
		}
		else if (45 < diff)
		{
			if (45 + 90 < diff)
				player->SetTurnAngle(look + 180.f);
			else
				player->SetTurnAngle(look + 90.f);

			stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		}
		player->SetSpineAngle(angle - look);
	}
}

CPlayerLeftTurn::CPlayerLeftTurn()
	: restAngle(90.f),
	lookAngle()
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
	restAngle = transform->GetRotation().y - player->GetTurnAngle();
	lookAngle = player->GetClickAngle();
	if (restAngle > 180) restAngle -= 180;
	else if (restAngle <= -270) restAngle += 360;
}

void CPlayerLeftTurn::Exit(CObject* object)
{
}

void CPlayerLeftTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 180.f * DT;
	if (restAngle < 0)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		transform->SetRotation(XMFLOAT3(0, player->GetTurnAngle(), 0));
	}
	else {
		float angle = transform->GetRotation().y - 180.f * DT;
		// cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}

	if (KEY_HOLD(KEY::RBUTTON))
	{
		RECT rect;
		GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
		XMFLOAT2 cursor = CURSOR;

		float centerX = (rect.right - rect.left) / 2;
		float centerY = (rect.bottom - rect.top) / 2;

		float x = cursor.x - centerX;
		float y = centerY - cursor.y;

		float angle = XMConvertToDegrees(atan2(x, y));
		lookAngle = angle;
		float look = transform->GetRotation().y;
		if (look > 180) look -= 360;
		player->SetSpineAngle(angle - look);
	}
	else if (!wasd)
	{
		float look = transform->GetRotation().y;
		player->SetSpineAngle(lookAngle - look);
	}
}

CPlayerRightTurn::CPlayerRightTurn()
	: restAngle(90.f),
	lookAngle()
{
}

CPlayerRightTurn::~CPlayerRightTurn()
{
}

void CPlayerRightTurn::Enter(CObject* object)
{
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);
	
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = player->GetTurnAngle() - transform->GetRotation().y;
	lookAngle = player->GetClickAngle();
	if (restAngle > 180) restAngle -= 180;
	else if (restAngle <= -270) restAngle += 360;
}

void CPlayerRightTurn::Exit(CObject* object)
{
}

void CPlayerRightTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 180.f * DT;
	if (restAngle < 0)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		transform->SetRotation(XMFLOAT3(0, player->GetTurnAngle(), 0));
	}
	else {
		float angle = transform->GetRotation().y + 180.f * DT;
		// cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}

	if (KEY_HOLD(KEY::RBUTTON))
	{
		RECT rect;
		GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
		XMFLOAT2 cursor = CURSOR;

		float centerX = (rect.right - rect.left) / 2;
		float centerY = (rect.bottom - rect.top) / 2;

		float x = cursor.x - centerX;
		float y = centerY - cursor.y;

		float angle = XMConvertToDegrees(atan2(x, y));
		float look = transform->GetRotation().y;
		if (look > 180) look -= 360;
		player->SetSpineAngle(angle - look);
	}
	else if (!wasd)
	{
		float look = transform->GetRotation().y;
		player->SetSpineAngle(lookAngle - look);
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
	//CS_WALK_ENTER_PACEKET send_packet;
	//send_packet.m_size = sizeof(CS_WALK_ENTER_PACEKET);
	//send_packet.m_type = PACKET_TYPE::P_CS_WALK_ENTER_PACKET;
	//PacketQueue::AddSendPacket(&send_packet);
	// cout << " >> send ) CS_WALK_ENTER_PACEKET" << endl;

	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("lisaWalk", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(400.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 400.0f * DT));
}

void CPlayerWalkState::Exit(CObject* object)
{
}

float Lerp(float A, float B, float Alpha)
{
	if (abs(A - B) > 180) B -= 360;
	float result = A * (1 - Alpha) + B * Alpha;
	return result;
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

	float angle = -45;
	if (KEY_HOLD(KEY::W)) {
		if (KEY_HOLD(KEY::D))
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 45 + angle, DT * 5), 0));
		else if (KEY_HOLD(KEY::A))
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 315 + angle, DT * 5), 0));
		else if (KEY_HOLD(KEY::S))
			stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		else {
			if (transform->GetRotation().y >= 180)
				transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 360 + angle, DT * 5), 0));
			else
				transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 0 + angle, DT * 5), 0));
		}
	}
	else if (KEY_HOLD(KEY::D)) {
		if (KEY_HOLD(KEY::S))
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 135 + angle, DT * 5), 0));
		else if (KEY_HOLD(KEY::A))
			stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		else
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 90 + angle, DT * 5), 0));
	}
	else if (KEY_HOLD(KEY::S)) {
		if (KEY_HOLD(KEY::A))
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 225 + angle, DT * 5), 0));
		else {
			transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 180 + angle, DT * 5), 0));
		}
	}
	else if (KEY_HOLD(KEY::A)) {
		transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, 270 + angle, DT * 5), 0));
	}



	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D))
	{
		rigidBody->AddForce(Vector3::ScalarProduct(transform->GetForward(), 15000.0f * DT));
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
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("Run", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(700.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 700.0f * DT));

}

void CPlayerRunState::Exit(CObject* object)
{
}

void CPlayerRunState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));

	if ((KEY_NONE(KEY::W) && KEY_NONE(KEY::S) && KEY_NONE(KEY::A) && KEY_NONE(KEY::D)) && Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	if (KEY_NONE(KEY::SHIFT) && (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D)))
	{
		stateMachine->ChangeState(CPlayerWalkState::GetInstance());
		return;
	}

	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::D))
	{
		rigidBody->AddForce(Vector3::ScalarProduct(XMFLOAT3(transform->GetForward()), 15000.0f * DT));
	}

	if (Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

}
