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
private:
	string	 m_spineName;
    CInventory* m_Inventory;
    vector<CUI*> m_UI;

public:
    vector<string> myItems;

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

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);

    void SetUI(CUI* ui);
    void SetNumber_of_items_UI(const string& ItemName, int prevItemsNum, int NextItemsNum);

    void GetItem(string item);
};