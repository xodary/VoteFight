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
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	CS_STATE_ENTER_PACKET p;
	p.m_size = sizeof(p);
	p.m_type = P_CS_STATE_ENTER_PACKET;
	p.m_pos = transform->GetPosition();
	p.m_state = (int)StateEnum::Idle;
	p.m_weapon = (int)player->m_Weapon;

	PacketQueue::AddSendPacket(&p);

	cout << "CPlayerIdleState" << endl;
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
		stateMachine->ChangeState(CPlayerWalkState::GetInstance());
		return;
	}

	if (KEY_TAP(KEY::LBUTTON))
	{
		player->Attack();
		//CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		//if(player->m_Weapon == WEAPON_TYPE::PUNCH) animator->Play("Punch", false);
		//else if(player->m_Weapon == WEAPON_TYPE::PISTOL) animator->Play("Pistol_shoot", false);
		//else if (player->m_Weapon == WEAPON_TYPE::AXE) animator->Play("Attack_onehand", false);
	}

	if (KEY_TAP(KEY::RBUTTON))
	{
		stateMachine->ChangeState(CPlayerFocusIdleState::GetInstance());
		return;
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
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	CS_STATE_ENTER_PACKET p;
	p.m_size = sizeof(p);
	p.m_type = P_CS_STATE_ENTER_PACKET;
	p.m_pos = transform->GetPosition();
	p.m_state = (int)StateEnum::Walk;
	p.m_weapon = (int)player->m_Weapon;

	PacketQueue::AddSendPacket(&p);

	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D) || KEY_HOLD(KEY::SHIFT))
	{
		XMFLOAT3 v(0, 0, 0);
		if (KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p.m_angle = vR;
		p.m_Rbutton = KEY_HOLD(KEY::RBUTTON);
		p.m_shift = KEY_HOLD(KEY::SHIFT);
		p.m_pos = transform->GetPosition();

		PacketQueue::AddSendPacket(&p);
	}

	cout << "CPlayerWalkState" << endl;
}

void CPlayerWalkState::Exit(CObject* object)
{
}

void CPlayerWalkState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (KEY_TAP(KEY::RBUTTON))
	{
		stateMachine->ChangeState(CPlayerFocusWalkState::GetInstance());
		return;
	}

	if (KEY_NONE(KEY::W) && KEY_NONE(KEY::A) && KEY_NONE(KEY::S) && KEY_NONE(KEY::D))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	if (KEY_TAP(KEY::S) || KEY_TAP(KEY::A) || KEY_TAP(KEY::W) || KEY_TAP(KEY::D) || KEY_TAP(KEY::SHIFT) ||
		KEY_AWAY(KEY::S) || KEY_AWAY(KEY::A) || KEY_AWAY(KEY::W) || KEY_AWAY(KEY::D) || KEY_AWAY(KEY::SHIFT))
	{
		XMFLOAT3 v(0, 0, 0);
		if (KEY_TAP(KEY::W) || KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_TAP(KEY::A) || KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_TAP(KEY::S) || KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_TAP(KEY::D) || KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p.m_angle = vR;
		p.m_pos = transform->GetPosition();
		p.m_Rbutton = KEY_HOLD(KEY::RBUTTON) || KEY_TAP(KEY::RBUTTON);
		p.m_shift = KEY_HOLD(KEY::SHIFT) || KEY_TAP(KEY::SHIFT);

		PacketQueue::AddSendPacket(&p);
	}

	if (KEY_HOLD(KEY::RBUTTON) || KEY_TAP(KEY::RBUTTON))
	{


		//player->SetClickAngle(angle);
		//float look = transform->GetRotation().y;
		//if (look > 180) look -= 360;
		//float diff = angle - look;
		//if (diff > 180) diff -= 360;
		//if (diff < -180) diff += 360;
		//if (-45 > diff)
		//{
		//	if (-90 - 45 > diff)
		//		player->SetTurnAngle(look - 180.f);
		//	else
		//		player->SetTurnAngle(look - 90.f);
		//	stateMachine->ChangeState(CPlayerLeftTurn::GetInstance());
		//}
		//else if (45 < diff)
		//{
		//	if (45 + 90 < diff)
		//		player->SetTurnAngle(look + 180.f);
		//	else
		//		player->SetTurnAngle(look + 90.f);

		//	stateMachine->ChangeState(CPlayerRightTurn::GetInstance());
		//}
		//player->SetSpineAngle(angle - look);
	}

}

CPlayerFocusIdleState::CPlayerFocusIdleState()
{
}

CPlayerFocusIdleState::~CPlayerFocusIdleState()
{
}

void CPlayerFocusIdleState::Enter(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	CS_STATE_ENTER_PACKET p;
	p.m_size = sizeof(p);
	p.m_type = P_CS_STATE_ENTER_PACKET;
	p.m_pos = transform->GetPosition();
	p.m_state = (int)StateEnum::FocusIdle;
	p.m_weapon = (int)player->m_Weapon;

	PacketQueue::AddSendPacket(&p);
}

void CPlayerFocusIdleState::Exit(CObject* object)
{
}

void CPlayerFocusIdleState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (KEY_AWAY(KEY::RBUTTON))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	if (KEY_HOLD(KEY::W) || KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::D))
	{
		stateMachine->ChangeState(CPlayerFocusWalkState::GetInstance());
		return;
	}

	if (KEY_TAP(KEY::LBUTTON))
	{
		//player->Attack();
		CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		if (player->m_Weapon == WEAPON_TYPE::PUNCH) animator->Play("Punch", false);
		else if (player->m_Weapon == WEAPON_TYPE::PISTOL) animator->Play("Pistol_shoot", false);
		else if (player->m_Weapon == WEAPON_TYPE::AXE) animator->Play("Attack_onehand", false);
	}

	RECT rect;
	GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
	XMFLOAT2 cursor = CURSOR;

	float centerX = (rect.right - rect.left) / 2;
	float centerY = (rect.bottom - rect.top) / 2;

	float x = cursor.x - centerX;
	float y = centerY - cursor.y;

	float angle = XMConvertToDegrees(atan2(x, y)) - 45;
	if (angle < 0) angle += 360;
	//player->goal_rota = angle;

	{
		CS_PLAYER_RBUTTON_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_PLAYER_RBUTTON_PACKET;
		p.m_angle = angle;
		PacketQueue::AddSendPacket(&p);
	}
}

CPlayerFocusWalkState::CPlayerFocusWalkState()
{
}

CPlayerFocusWalkState::~CPlayerFocusWalkState()
{
}

void CPlayerFocusWalkState::Enter(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	CS_STATE_ENTER_PACKET p;
	p.m_size = sizeof(p);
	p.m_type = P_CS_STATE_ENTER_PACKET;
	p.m_pos = transform->GetPosition();
	p.m_state = (int)StateEnum::FocusWalk;
	p.m_weapon = (int)player->m_Weapon;

	PacketQueue::AddSendPacket(&p);

	if (KEY_HOLD(KEY::S) || KEY_HOLD(KEY::A) || KEY_HOLD(KEY::W) || KEY_HOLD(KEY::D) || KEY_HOLD(KEY::SHIFT))
	{
		XMFLOAT3 v(0, 0, 0);
		if (KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p.m_angle = vR;
		p.m_pos = transform->GetPosition();
		p.m_Rbutton = KEY_HOLD(KEY::RBUTTON);
		p.m_shift = KEY_HOLD(KEY::SHIFT);

		PacketQueue::AddSendPacket(&p);
	}

	cout << "CPlayerFocusState" << endl;
}

void CPlayerFocusWalkState::Exit(CObject* object)
{
}

void CPlayerFocusWalkState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
	
	if (KEY_AWAY(KEY::RBUTTON))
	{
		stateMachine->ChangeState(CPlayerIdleState::GetInstance());
		return;
	}

	if (KEY_NONE(KEY::W) && KEY_NONE(KEY::A) && KEY_NONE(KEY::S) && KEY_NONE(KEY::D))
	{
		stateMachine->ChangeState(CPlayerFocusIdleState::GetInstance());
		return;
	}

	RECT rect;
	GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);
	XMFLOAT2 cursor = CURSOR;

	float centerX = (rect.right - rect.left) / 2;
	float centerY = (rect.bottom - rect.top) / 2;

	float x = cursor.x - centerX;
	float y = centerY - cursor.y;

	float angle = XMConvertToDegrees(atan2(x, y)) - 45;
	if (angle < 0) angle += 360;
	//player->goal_rota = angle;

	{
		CS_PLAYER_RBUTTON_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_PLAYER_RBUTTON_PACKET;
		p.m_angle = angle;
		PacketQueue::AddSendPacket(&p);
	}

	if (KEY_TAP(KEY::S) || KEY_TAP(KEY::A) || KEY_TAP(KEY::W) || KEY_TAP(KEY::D) || KEY_TAP(KEY::SHIFT) ||
		KEY_AWAY(KEY::S) || KEY_AWAY(KEY::A) || KEY_AWAY(KEY::W) || KEY_AWAY(KEY::D) || KEY_AWAY(KEY::SHIFT))
	{
		XMFLOAT3 v(0, 0, 0);
		if (KEY_TAP(KEY::W) || KEY_HOLD(KEY::W)) v.z -= 1.0f;
		if (KEY_TAP(KEY::A) || KEY_HOLD(KEY::A)) v.x -= 1.0f;
		if (KEY_TAP(KEY::S) || KEY_HOLD(KEY::S)) v.z += 1.0f;
		if (KEY_TAP(KEY::D) || KEY_HOLD(KEY::D)) v.x += 1.0f;
		if (Vector3::IsZero(v)) return;
		v = Vector3::Normalize(v);
		float vR = XMConvertToDegrees(atan2(v.z, v.x)) + 45;
		if (vR < 0) vR += 360;

		CS_VELOCITY_CHANGE_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_VELOCITY_CHANGE_PACKET;
		p.m_angle = vR;
		p.m_pos = transform->GetPosition();
		p.m_Rbutton = KEY_HOLD(KEY::RBUTTON);
		p.m_shift = KEY_HOLD(KEY::SHIFT);

		PacketQueue::AddSendPacket(&p);
	}
}