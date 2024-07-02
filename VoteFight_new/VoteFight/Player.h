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

    int getItems(const string& ItemName) {
        for (const auto& item : items) {
            if (item.first == ItemName) {
                return item.second;
            }
        }
        return 0; // 해당 아이템이 발견되지 않은 경우
    }
};
