#pragma once
#include "Object.h"

class CBilboardUI;

class CCharacter : public CObject
{
public:
	int      m_health;
	bool	 m_dead;
	unsigned int		 m_damageType;

	
	CCharacter();
	virtual ~CCharacter();

	void SetHealth(int health);
	int GetHealth();

	void SetWeapon(CObject* object);
	CObject* GetWeapon();

	bool IsEquippedWeapon();
	virtual void SwapWeapon(WEAPON_TYPE weaponType);
	virtual void Update();
};
