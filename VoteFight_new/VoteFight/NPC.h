#pragma once
#include "Character.h"

class CQuest;
class C_NPC_Item;
class CBilboardUI;

class CNPC : public CCharacter
{
private:
	string	 m_spineName;
	float	 m_spineAngle;
	float    m_turnAngle;

   CQuest* m_Quest;

   CBilboardUI* speech_bubble;
public:
	CNPC();
	virtual ~CNPC();
	
	string GetSpineName();
	float GetSpineAngle();
	void SetSpineAngle(float angle);

    CQuest* GetQuest() { return m_Quest; };

	virtual void Init();
	virtual void Update();
	virtual void OnCollisionEnter(CObject* collidedObject);
};

class C_NPC_Item {
private:
    std::string itemName;
    int quantity;

public:
    C_NPC_Item() {};
    C_NPC_Item(const std::string& name, int qty) : itemName(name), quantity(qty) {}

    std::string getName() const {
        return itemName;
    }

    int getQuantity() const {
        return quantity;
    }
};


class CQuest {
private:
    std::string questName;
    std::string questDescription;
    C_NPC_Item m_questItem;
    bool isCompleted = false;

public:
    // 생성자
    CQuest() {};
    CQuest(const std::string& name, const std::string& description, const C_NPC_Item& item)
        : questName(name), questDescription(description), m_questItem(item), isCompleted(false) {}


    // 소멸자
    ~CQuest() {}

    // 퀘스트 이름 반환
    std::string getName() const {
        return questName;
    }

    // 퀘스트 설명 반환
    std::string getDescription() const {
        return questDescription;
    }

    // 퀘스트 완료 여부 반환
    bool getCompletionStatus() const {
        return isCompleted;
    }

    // 퀘스트 완료 처리
    void complete() {
        isCompleted = true;
    }

    string GetItemName() const { return m_questItem.getName(); };
    UINT GetItemQuantity()const { return m_questItem.getQuantity(); };

    void show()
    {
        std::cout << "퀘스트 이름: " << questName << std::endl;
        std::cout << "퀘스트 설명: " << questDescription << std::endl;
        std::cout << "퀘스트 아이템: " << m_questItem.getName() << " (수량: " << m_questItem.getQuantity() << ")" << std::endl;
    }
};

