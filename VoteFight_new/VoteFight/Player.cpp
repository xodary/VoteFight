#include "pch.h"
#include "Player.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "Camera.h"
#include "StateMachine.h"
#include "Transform.h"
#include "PlayerStates.h"

CPlayer::CPlayer() :
	m_isAiming(),
	m_bulletCount(),
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("Player");
	m_Inventory = new CInventory();
}

CPlayer::~CPlayer()
{
}

void CPlayer::SetClickAngle(float clickAngle)
{
	m_clickAngle = clickAngle;
}

float CPlayer::GetClickAngle()
{
	return m_clickAngle;
}

void CPlayer::SetTurnAngle(float look)
{
	m_turnAngle = look;
}

float CPlayer::GetTurnAngle()
{
	return m_turnAngle;
}

string CPlayer::GetSpineName()
{
	return m_spineName;
}

float CPlayer::GetSpineAngle()
{
	return m_spineAngle;
}

void CPlayer::SetSpineAngle(float angle)
{
	m_spineAngle = angle;
}

void CPlayer::SetAiming(bool isAiming)
{
	m_isAiming = isAiming;
}

bool CPlayer::IsAiming()
{
	return m_isAiming;
}

bool CPlayer::HasBullet()
{
	return m_bulletCount > 0;
}

void CPlayer::Init()
{
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CPlayerIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->SetWeight("idle", 1.0f);
	m_Inventory = new CInventory();

}

void CPlayer::SwapWeapon(WEAPON_TYPE weaponType)
{
}

void CPlayer::Punch()
{
}

void CPlayer::Shoot()
{
}

void CPlayer::Update()
{
	CObject::Update();
	
}

void CPlayer::OnCollisionEnter(CObject* collidedObject)
{
	cout << collidedObject->GetName() << endl;
	/*
	m_Inventory->addItem(CItem(collidedObject->GetName(), 1));
	collidedObject->SetDeleted(true);
	m_Inventory->displayInventory();
	*/
}



//
/// 아이템 및 인벤토리 

CInventory::CInventory()
{
}

CInventory::~CInventory()
{
}
