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
    vector<CBilboardUI*> m_bilboardUI;

public:
    bool isMove = true;
	CPlayer();
	virtual ~CPlayer();

	virtual void Init();
    virtual void AnotherInit();

    void Attack();
	virtual void SwapWeapon(WEAPON_TYPE weaponType);

	void Punch();
	void Shoot();

	virtual void Update();
	virtual void RenderBilboard(CCamera* camera);

    unsigned int    m_id{};

    WEAPON_TYPE m_Weapon;

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
