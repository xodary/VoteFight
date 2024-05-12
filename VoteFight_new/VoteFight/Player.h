#pragma once
#include "Character.h"

class CItem;
class CInventory;

class CPlayer : public CCharacter
{
private:
	bool	 m_isAiming;
	int	     m_bulletCount;
	string	 m_spineName;
	float	 m_spineAngle;
	float    m_turnAngle;
	float	 m_clickAngle;

    CInventory* m_Inventory;
public:
	CPlayer();
	virtual ~CPlayer();

	void SetClickAngle(float clickAngle);
	float GetClickAngle();
	void SetTurnAngle(float look);
	float GetTurnAngle();
	string GetSpineName();
	float GetSpineAngle();
	void SetSpineAngle(float angle);
	void SetAiming(bool isAiming);
	bool IsAiming();

	bool HasBullet();

	virtual void Init();
    virtual void AnotherInit();

	virtual void SwapWeapon(WEAPON_TYPE weaponType);

	void Punch();
	void Shoot();

	virtual void Update();

    unsigned int    m_id{};

	bool b_front = false;
	bool b_right = false;
	bool b_back = false;
	bool b_left = false;
	//void ManagePistol(bool HasPistol);
	//bool HasPistol() const;
	//bool IsEquippedPistol() const;

	//void ManageKey(bool HasKey);
	//bool HasKey() const;

	//bool SwapWeapon(WEAPON_TYPE WeaponType);

	//void Rotate(float Pitch, float Yaw, float Roll, float ElapsedTime, float NearestHitDistance);

	//void IsCollidedByEventTrigger(const XMFLOAT3& NewPosition);

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);
};

class CItem {
private:
    string name;
    int quantity;

public:
    CItem(string itemName, int itemQuantity) : name(itemName), quantity(itemQuantity) {}

    // 아이템 이름 반환
    string getName() const {
        return name;
    }

    // 아이템 수량 반환
    int getQuantity() const {
        return quantity;
    }

    // 아이템 수량 증가
    void increaseQuantity(int amount) {
        quantity += amount;
    }
};

// 인벤토리 클래스
class CInventory {
private:
    vector<pair<string, int>> items;

public:
    CInventory();
    virtual ~CInventory();

    // 아이템 추가
    void addItem(const string& itemName, int quantity) {
        // 아이템 중복 검사 및 수량 증가
        for (auto& item : items) {
            if (item.first == itemName) {
                item.second += quantity;
                return;
            }
        }

        // 아이템이 존재하지 않으면 추가
        items.push_back(make_pair(itemName, quantity));
    }

    // 아이템 삭제
    void deleteItem(const string& itemName, int quantity) {
        // 아이템 검색 및 수량 감소
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->first == itemName) {
                it->second -= quantity;
                if (it->second <= 0) {
                    items.erase(it);
                }
                return;
            }
        }
    }

    // 아이템 교환
    bool exchangeItem(const string& itemName, int quantity, const string& newItemName, int newQuantity);

    // 인벤토리 출력
    void displayInventory() const {
        cout << "Inventory:" << endl;
        for (const auto& item : items) {
            cout << "아이템 이름 : " << item.first << ", 개수: " << item.second << endl;
        }
    }
};
