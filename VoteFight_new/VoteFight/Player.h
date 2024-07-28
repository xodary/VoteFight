#pragma once
#include "Character.h"

class CItem;
class CInventory;
class CUI;
class CTextUI;
class CBilboardUI;
class CWeapon;

class CPlayer : public CCharacter
{
public:
    CInventory*		m_Inventory;
    vector<CUI*>	m_UI;
	int				m_tickets;

    vector<CItem>	myItems;
	bool			reloading;

    float goal_rota;

    bool isMove = true;
	CPlayer();
	virtual ~CPlayer();

	virtual void Init();

    void Attack();
	virtual void SwapWeapon(WEAPON_TYPE weaponType);

	void Punch();
	void Shoot(CScene& currScene);

	virtual void Update();

    WEAPON_TYPE m_Weapon;
	int			m_FullBullets = 10;
	int			m_bullets = 10;

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);

    void GetItem(string name, int capacity);
};

class CItem
{
public:
	string	m_name;
	int		m_capacity;
};