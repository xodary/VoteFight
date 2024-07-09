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
#include "Weapon.h"
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
	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	rigidBody->m_velocity = XMFLOAT3(0, 0, 0);
}

void CPlayerIdleState::Exit(CObject* object)
{
}

void CPlayerIdleState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D))
	{
		if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D))
		{
			stateMachine->ChangeState(CPlayerWalkState::GetInstance());
			return;
		}
	}

	if (KEY_TAP(KEY::LBUTTON))
	{
		//player->Attack();
		CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		if(player->m_Weapon == WEAPON_TYPE::PUNCH) animator->Play("Punch", false);
		else if(player->m_Weapon == WEAPON_TYPE::PISTOL) animator->Play("Pistol_shoot", false);
		else if (player->m_Weapon == WEAPON_TYPE::AXE) animator->Play("Attack_onehand", false);
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
	//CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	//if (player->m_Weapon == WEAPON_TYPE::PUNCH) animator->Play("Run", true);
	//else animator->Play("Pistol_run", true);

	//CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	//CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	//rigidBody->SetMaxSpeedXZ(150.0f);
	//rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 3000.0f * DT));
	
	CPlayer* player = static_cast<CPlayer*>(object);

	CS_WALK_ENTER_PACKET p;
	p.m_size = sizeof(p);
	p.m_type = P_CS_WALK_ENTER_PACKET;
	p.m_weapon = player->m_Weapon;

	PacketQueue::AddSendPacket(&p);

	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D) || KEY_HOLD(KEY::SHIFT))
	{
		CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

		XMFLOAT3 v(0, 0, 0);
		if (KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p2;
		p2.m_size = sizeof(p2);
		p2.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p2.m_rota = vR;
		p2.m_shift = KEY_HOLD(KEY::SHIFT);
		p2.m_pos = transform->GetPosition();

		PacketQueue::AddSendPacket(&p2);
	}

}

void CPlayerWalkState::Exit(CObject* object)
{
}

void CPlayerWalkState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	//CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	//CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));

	if (KEY_NONE(KEY::W) && KEY_NONE(KEY::A) && KEY_NONE(KEY::S) && KEY_NONE(KEY::D))
	{
		CS_STOP_PACKET p2;
		p2.m_size = sizeof(p2);
		p2.m_type = P_CS_STOP_PACKET;

		PacketQueue::AddSendPacket(&p2);

		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	//if (KEY_TAP(KEY::SHIFT))
	//{
	//	rigidBody->SetMaxSpeedXZ(300.0f);
	//	//rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 4000.0f * DT));
	//	animator->SetSpeed("Pistol_run", 2.0f);
	//	animator->SetSpeed("Run", 2.0f);
	//}
	//else if (KEY_AWAY(KEY::SHIFT))
	//{
	//	rigidBody->SetMaxSpeedXZ(150.0f);
	//	//rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 3000.0f * DT));
	//	animator->SetSpeed("Pistol_run", 1.0f);
	//	animator->SetSpeed("Run", 1.0f);
	//}

	if (KEY_TAP(KEY::S) || KEY_TAP(KEY::A) || KEY_TAP(KEY::W) || KEY_TAP(KEY::D) || KEY_TAP(KEY::SHIFT) ||
		KEY_AWAY(KEY::S) || KEY_AWAY(KEY::A) || KEY_AWAY(KEY::W) || KEY_AWAY(KEY::D) || KEY_AWAY(KEY::SHIFT))
	{
		CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

		XMFLOAT3 v(0, 0, 0);
		if (KEY_TAP(KEY::W) || KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_TAP(KEY::A) || KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_TAP(KEY::S) || KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_TAP(KEY::D) || KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p2;
		p2.m_size = sizeof(p2);
		p2.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p2.m_rota = vR;
		p2.m_shift = KEY_TAP(KEY::SHIFT) || KEY_HOLD(KEY::SHIFT);
		p2.m_pos = transform->GetPosition();

		PacketQueue::AddSendPacket(&p2);
	}
	//XMFLOAT3 vector = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, vR, 0)));
	//if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D))
	//{
	//	rigidBody->AddForce(Vector3::ScalarProduct(vector, 3000.0f * DT), player->isMove);
	//}

	//if (Math::IsZero(rigidBody->GetSpeedXZ()))
	//{
	//	stateMachine->ChangeState(CPlayerIdleState::GetInstance());
	//	return;
	//}
}