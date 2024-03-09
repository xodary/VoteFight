#include "pch.h"
#include "Character.h"

#include "TimeManager.h"

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
    // 무기를 소유하고 있다면, 활성화 여부에 따라 장착 여부가 결정된다.
    if (m_weapon != nullptr)
    {
        return m_weapon->IsActive();
    }

    // 무기가 없다면 false를 반환한다.
    return false;
}

void CCharacter::SwapWeapon(WEAPON_TYPE weaponType)
{
    if (m_weapon != nullptr)
    {
        switch (weaponType)
        {
        case WEAPON_TYPE::PUNCH:
            m_weapon->SetActive(false);
            break;
        case WEAPON_TYPE::PISTOL:
            m_weapon->SetActive(true);
            break;
        }
    }
}
