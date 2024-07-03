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
    // ������
    CQuest() {};
    CQuest(const std::string& name, const std::string& description, const C_NPC_Item& item)
        : questName(name), questDescription(description), m_questItem(item), isCompleted(false) {}


    // �Ҹ���
    ~CQuest() {}

    // ����Ʈ �̸� ��ȯ
    std::string getName() const {
        return questName;
    }

    // ����Ʈ ���� ��ȯ
    std::string getDescription() const {
        return questDescription;
    }

    // ����Ʈ �Ϸ� ���� ��ȯ
    bool getCompletionStatus() const {
        return isCompleted;
    }

    // ����Ʈ �Ϸ� ó��
    void complete() {
        isCompleted = true;
    }

    string GetItemName() const { return m_questItem.getName(); };
    UINT GetItemQuantity()const { return m_questItem.getQuantity(); };

    void show()
    {
        std::cout << "����Ʈ �̸�: " << questName << std::endl;
        std::cout << "����Ʈ ����: " << questDescription << std::endl;
        std::cout << "����Ʈ ������: " << m_questItem.getName() << " (����: " << m_questItem.getQuantity() << ")" << std::endl;
    }
};

