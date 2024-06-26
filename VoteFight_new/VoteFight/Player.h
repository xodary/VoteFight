#pragma once
#include "Character.h"

class CItem;
class CInventory;
class CUI;
class CTextUI;

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
    vector<CUI*> m_UI;

    CTextUI* m_playerTextBar;
public:
    bool isMove = true;
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
	virtual void RenderBilboard(CCamera* camera);

    unsigned int    m_id{};

    float    m_goalAngle;

	//void ManagePistol(bool HasPistol);
	//bool HasPistol() const;
	//bool IsEquippedPistol() const;

	//void ManageKey(bool HasKey);
	//bool HasKey() const;

	//bool SwapWeapon(WEAPON_TYPE WeaponType);

	//void Rotate(float Pitch, float Yaw, float Roll, float ElapsedTime, float NearestHitDistance);

	//void IsCollidedByEventTrigger(const XMFLOAT3& NewPosition);

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);

    void SetUI(CUI* ui);
    void SetNumber_of_items_UI(const string& ItemName, int prevItemsNum, int NextItemsNum);
};

class CItem {
private:
    string name;
    int quantity;

public:
    CItem(string itemName, int itemQuantity) : name(itemName), quantity(itemQuantity) {}

    // ������ �̸� ��ȯ
    string getName() const {
        return name;
    }

    // ������ ���� ��ȯ
    int getQuantity() const {
        return quantity;
    }

    // ������ ���� ����
    void increaseQuantity(int amount) {
        quantity += amount;
    }
};

// �κ��丮 Ŭ����
class CInventory {
private:
    vector<pair<string, int>> items;

public:
    CInventory();
    virtual ~CInventory();

    // ������ �߰�
    void addItem(const string& itemName, int quantity) {
        // ������ �ߺ� �˻� �� ���� ����
        for (auto& item : items) {
            if (item.first == itemName) {
                item.second += quantity;
                return;
            }
        }

        // �������� �������� ������ �߰�
        items.push_back(make_pair(itemName, quantity));
    }

    // ������ ����
    void deleteItem(const string& itemName, int quantity) {
        // ������ �˻� �� ���� ����
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

    // ������ ��ȯ
    bool exchangeItem(const string& itemName, int quantity, const string& newItemName, int newQuantity);

    // �κ��丮 ���
    void displayInventory() const {
        cout << "Inventory:" << endl;
        for (const auto& item : items) {
            cout << "������ �̸� : " << item.first << ", ����: " << item.second << endl;
        }
    }

    int getItems(const string& ItemName) {
        for (const auto& item : items) {
            if (item.first == ItemName) {
                return item.second;
            }
        }
        return 0; // �ش� �������� �߰ߵ��� ���� ���
    }
};
