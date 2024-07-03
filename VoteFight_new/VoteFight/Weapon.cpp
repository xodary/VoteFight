#include "pch.h"
#include "Animator.h"
#include "Weapon.h"

CWeapon::CWeapon()
{
}

CWeapon::~CWeapon()
{
}

CGun::CGun()
{
	m_nDamage = 10;
	m_nBullet = 15;
}

CGun::~CGun()
{
}

void CGun::Attack(CObject* object)
{
}

CSword::CSword()
{
	m_nDamage = 20;
}

CSword::~CSword()
{
}

void CSword::Attack(CObject* object)
{
}

