#pragma once
#include "Object.h"

class CBilboardUI;

class CCharacter : public CObject
{
private:
	int      m_health;

	CObject* m_weapon;

public:
	vector<CBilboardUI*> m_bilboardUI;
	
	CCharacter();
	virtual ~CCharacter();

	void SetHealth(int health);
	int GetHealth();

	void SetWeapon(CObject* object);
	CObject* GetWeapon();

	bool IsEquippedWeapon();
	virtual void SwapWeapon(WEAPON_TYPE weaponType);
	void RenderBilboard(CCamera* camera);
	virtual void Update();
};
