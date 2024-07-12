#pragma once
#include "Object.h"

class CBilboardUI;

class CCharacter : public CObject
{
private:
	int      m_health;

	CObject* m_weapon;

public:
	
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
