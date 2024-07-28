#include "pch.h"
#include "Character.h"
#include "TimeManager.h"
#include "UI.h"
#include "RigidBody.h"
#include "Transform.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"

CCharacter::CCharacter() :
    m_health(100),
    m_dead(false)
{
    CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
    CreateComponent(COMPONENT_TYPE::RIGIDBODY);
}

CCharacter::~CCharacter()
{
}

void CCharacter::SetHealth(int health)
{
    //m_health = clamp(health, 0, 100);
    m_health = health;
    if (m_health <= 0) m_dead = true;
}

int CCharacter::GetHealth()
{
    return m_health;
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