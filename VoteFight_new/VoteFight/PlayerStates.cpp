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
	stateNum = STATE_ENUM::CPlayerIdleState;
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

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D))
	{
		XMFLOAT3 v2(0, 0, 0);
		if (KEY_HOLD(KEY::W)) {
			v2.x -= 1.0f;
			v2.z += 1.0f;
		}
		if (KEY_HOLD(KEY::A)) {
			v2.x -= 1.0f;
			v2.z -= 1.0f;
		}
		if (KEY_HOLD(KEY::S)) {
			v2.x += 1.0f;
			v2.z -= 1.0f;
		}
		if (KEY_HOLD(KEY::D)) {
			v2.x += 1.0f;
			v2.z += 1.0f;
		}
		v2 = Vector3::Normalize(v2);
		XMFLOAT3 v1 = Vector3::TransformCoord(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(transform->GetRotation()));
		v1 = XMFLOAT3(v1.x, 0.0f, v1.z);
		float v1R = atan2(v1.z, v1.x);
		float v2R = atan2(v2.z, v2.x);
		float nowRotation = XMConvertToDegrees(v1R - v2R);
		if (nowRotation > 180) nowRotation -= 360;
		if (nowRotation < -180) nowRotation += 360;
		player->SetTurnAngle(nowRotation);
		player->m_goalAngle = transform->GetRotation().y + nowRotation;
		if (player->m_goalAngle > 180) player->m_goalAngle -= 360;
		if (player->m_goalAngle < -180) player->m_goalAngle += 360;

		if(abs(nowRotation) < 5)
			stateMachine->ChangeState(CPlayerWalkState::GetInstance());
		else if (nowRotation > 0)
			stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		else if (nowRotation < 0)
			stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
			
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
		angle -= 45;
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
	stateNum = STATE_ENUM::CPlayerLeftTurn;

	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);

	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = abs(player->GetTurnAngle());
	lookAngle = player->GetClickAngle();
}

void CPlayerLeftTurn::Exit(CObject* object)
{
}

void CPlayerLeftTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 220.f * DT;
	if (restAngle < 0)
	{
		transform->SetRotation(XMFLOAT3(0, player->m_goalAngle, 0));
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
	}
	else {
		float angle = transform->GetRotation().y - 220.f * DT;
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
	stateNum = STATE_ENUM::CPlayerRightTurn;

	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);
	
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = abs(player->GetTurnAngle());
	lookAngle = player->GetClickAngle();
}

void CPlayerRightTurn::Exit(CObject* object)
{
}

void CPlayerRightTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 220.f * DT;
	if (restAngle < 0)
	{
		transform->SetRotation(XMFLOAT3(0, player->m_goalAngle, 0));
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
	}
	else {
		float angle = transform->GetRotation().y + 220.f * DT;
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

	stateNum = STATE_ENUM::CPlayerWalkState;
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("lisaWalk", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(300.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 300.0f * DT));
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

	XMFLOAT3 v2(0, 0, 0);
	if (KEY_HOLD(KEY::W)) {
		v2.x -= 1.0f;
		v2.z += 1.0f;
	}
	if (KEY_HOLD(KEY::A)) {
		v2.x -= 1.0f;
		v2.z -= 1.0f;
	}
	if (KEY_HOLD(KEY::S)) {
		v2.x += 1.0f;
		v2.z -= 1.0f;
	}
	if (KEY_HOLD(KEY::D)) {
		v2.x += 1.0f;
		v2.z += 1.0f;
	}
	if (Vector3::IsZero(v2)) return;
	v2 = Vector3::Normalize(v2);
	XMFLOAT3 v1 = Vector3::TransformCoord(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(transform->GetRotation()));
	v1 = XMFLOAT3(v1.x, 0.0f, v1.z);
	float v1R = atan2(v1.z, v1.x);
	float v2R = atan2(v2.z, v2.x);
	float nowRotation = XMConvertToDegrees(v1R - v2R);
	if (nowRotation > 180) nowRotation -= 360;
	if (nowRotation < -180) nowRotation += 360;
	float m_goalAngle = transform->GetRotation().y + nowRotation;
	if (player->m_goalAngle > 180) player->m_goalAngle -= 360;
	if (player->m_goalAngle < -180) player->m_goalAngle += 360;
	transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, m_goalAngle, DT * 8), 0));

	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D))
	{
		rigidBody->AddForce(Vector3::ScalarProduct(transform->GetForward(), 5000.0f * DT));
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
	stateNum = STATE_ENUM::CPlayerRunState;
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("Run", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(400.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 400.0f * DT));

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

	XMFLOAT3 v2(0, 0, 0);
	if (KEY_HOLD(KEY::W)) {
		v2.x -= 1.0f;
		v2.z += 1.0f;
	}
	if (KEY_HOLD(KEY::A)) {
		v2.x -= 1.0f;
		v2.z -= 1.0f;
	}
	if (KEY_HOLD(KEY::S)) {
		v2.x += 1.0f;
		v2.z -= 1.0f;
	}
	if (KEY_HOLD(KEY::D)) {
		v2.x += 1.0f;
		v2.z += 1.0f;
	}
	if (Vector3::IsZero(v2)) return;
	v2 = Vector3::Normalize(v2);
	XMFLOAT3 v1 = Vector3::TransformCoord(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(transform->GetRotation()));
	v1 = XMFLOAT3(v1.x, 0.0f, v1.z);
	float v1R = atan2(v1.z, v1.x);
	float v2R = atan2(v2.z, v2.x);
	float nowRotation = XMConvertToDegrees(v1R - v2R);
	if (nowRotation > 180) nowRotation -= 360;
	if (nowRotation < -180) nowRotation += 360;
	float m_goalAngle = transform->GetRotation().y + nowRotation;
	if (player->m_goalAngle > 180) player->m_goalAngle -= 360;
	if (player->m_goalAngle < -180) player->m_goalAngle += 360;
	transform->SetRotation(XMFLOAT3(0, Lerp(transform->GetRotation().y, m_goalAngle, DT * 8), 0));

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::D))
	{
		rigidBody->AddForce(Vector3::ScalarProduct(XMFLOAT3(transform->GetForward()), 8000.0f * DT));
	}

	if (Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

}
