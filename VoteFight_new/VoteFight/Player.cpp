#include "pch.h"
#include "Player.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Camera.h"
#include "StateMachine.h"
#include "Transform.h"
#include "PlayerStates.h"

CPlayer::CPlayer() :
	m_isAiming(),
	m_bulletCount()
{
	SetName("Player");
}

CPlayer::~CPlayer()
{
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
