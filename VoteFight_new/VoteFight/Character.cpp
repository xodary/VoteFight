#include "pch.h"
#include "Character.h"
#include "TimeManager.h"
#include "UI.h"
#include "RigidBody.h"
#include "Transform.h"

CCharacter::CCharacter() :
    m_health(100),
    m_weapon()
{
    CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
    CreateComponent(COMPONENT_TYPE::RIGIDBODY);
}

CCharacter::~CCharacter()
{
}

void CCharacter::SetHealth(int health)
{
    m_health = clamp(health, 0, 100);
}

int CCharacter::GetHealth()
{
    return m_health;
}

void CCharacter::SetWeapon(CObject* object)
{
    m_weapon = object;
}

CObject* CCharacter::GetWeapon()
{
    return m_weapon;
}

bool CCharacter::IsEquippedWeapon()
{
    return true;
}

void CCharacter::SwapWeapon(WEAPON_TYPE weaponType)
{
}

void CCharacter::Update()
{
    CObject::Update();
}