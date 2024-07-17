#include "pch.h"
#include "Player.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "RigidBody.h"
#include "SceneManager.h"
#include "Scene.h"
#include "AssetManager.h"
#include "Weapon.h"
#include "Material.h"
#include "Animator.h"
#include "Camera.h"
#include "StateMachine.h"
#include "Transform.h"
#include "PlayerStates.h"
#include "NPC.h"
#include "UI.h"
#include "Bullet.h"
#include "Transform.h"
#include "Mesh.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"

CPlayer::CPlayer() : m_spineName("mixamorig:Spine")
{
	SetName("Player");
	m_Inventory = new CInventory();
	SetGroupType((UINT)GROUP_TYPE::PLAYER);
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	CAnimator* animator = reinterpret_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	//animator->SetWeight("Idle", 1.0f);
	//animator->Play("Idle", true);
	animator->SetWeight("Pistol_slowwalk", 1.0f);
	animator->Play("Pistol_slowwalk", true);

	m_Inventory = new CInventory();

	myItems.resize(18);
	myItems[0] = "wood";
	
	m_bilboardUI.emplace_back(new CHPbarUI(this));
	m_bilboardUI.push_back(new CTextUI(this));
	m_bilboardUI.push_back(new CSpeechBubbleUI(this));
}

void CPlayer::Attack()
{
	//m_Weapon->Attack(this);

	if (CSceneManager::GetInstance()->GetCurrentScene()->GetName() == "GameScene")
	{
		CS_ATTACK_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_ATTACK_PACKET;
		p.m_weapon = (int)m_Weapon;
		p.m_angle = GetRotate().y;
		p.m_pos = FindFrame("GunPos")->GetPosition();
		p.m_Rbutton = KEY_HOLD(KEY::RBUTTON);
		//p.m_pos = GetPosition();

		PacketQueue::AddSendPacket(&p);
	}
}

void CPlayer::SwapWeapon(WEAPON_TYPE weaponType)
{
	m_Weapon = weaponType;
	CObject* GunPos = FindFrame("GunPos");
	CObject* AxePos = FindFrame("AxePos");
	if (GunPos->m_children.size() > 0) delete GunPos->m_children[0]; GunPos->m_children.clear();
	if (AxePos->m_children.size() > 0) delete AxePos->m_children[0]; AxePos->m_children.clear();

	switch (weaponType) {
	case WEAPON_TYPE::PISTOL:
	{
		CGun* weapon = reinterpret_cast<CGun*>(CObject::Load("Gun"));
		CObject* pos = FindFrame("GunPos");
		pos->m_children.push_back(weapon);
		weapon->m_parent = pos;
	}
	break;
	case WEAPON_TYPE::AXE:
	{
		CGun* weapon = reinterpret_cast<CGun*>(CObject::Load("axe"));
		CObject* pos = FindFrame("AxePos");
		pos->m_children.push_back(weapon);
		weapon->m_parent = pos;
	}
	break;
	}

}

void CPlayer::Punch()
{
}

//void CPlayer::Shoot(CScene& currScene)
//{
//	if (currScene.GetName() == "GameScene")
//	{
//		cout << "in GameScene" << endl;
//		CBullet* Bullet = reinterpret_cast<CBullet*>(Load("Bullet"));
//		if (Bullet) {
//			CObject* pos = FindFrame("Gun");
//			XMFLOAT3 currPostion(pos->GetPosition());
//			Bullet->SetPostion(currPostion);
//			Bullet->SetRotate(GetRotate());
//			currScene.AddObject(GROUP_TYPE::BULLET, Bullet, 0);
//			Bullet->Shoot();
//		}
//	}
//}

void CPlayer::Update()
{
	CCharacter::Update();
}

void CPlayer::OnCollisionEnter(CObject* collidedObject)
{
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::NPC:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "Pick")
				m_UI[i]->SetActive(true);
		}
		break;

	case (UINT)GROUP_TYPE::BULLET:
		cout << "��Ʈ!" << endl;
		break;
	case (UINT)GROUP_TYPE::GROUND_ITEM:
		GetItem(collidedObject->GetName());
		CSceneManager::GetInstance()->GetCurrentScene()->DeleteObject(GROUP_TYPE::GROUND_ITEM, collidedObject->m_id);
		break;
	}
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::STRUCTURE:
	case (UINT)GROUP_TYPE::BOX:
		CRigidBody* rigidBody = static_cast<CRigidBody*>(GetComponent(COMPONENT_TYPE::RIGIDBODY));
		rigidBody->ReturnPrevLocation(rigidBody->GetVelocity());
		break;
	}
}
void CPlayer::OnCollision(CObject* collidedObject)
{
	if (collidedObject->GetName() == "PP_Tree_02") {
		if (KEY_TAP(KEY::LBUTTON) && m_Weapon == WEAPON_TYPE::AXE) {
			GetItem("wood");
		}
	}

	if (KEY_TAP(KEY::F))
	{
		if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::STRUCTURE)
		{
			m_Inventory->addItem(collidedObject->GetName(), 1);
			collidedObject->SetDeleted(true);
		}
		if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::NPC)
		{
			//CNPC* targetNPC = (CNPC*)collidedObject;
			//if (!targetNPC->GetQuest()->getCompletionStatus())
			//{
			//	SetNumber_of_items_UI("tiket", m_Inventory->getItems("tiket"), m_Inventory->getItems("tiket") + 1);
			//	m_Inventory->exchangeItem(targetNPC->GetQuest()->GetItemName(), targetNPC->GetQuest()->GetItemQuantity(), "tiket", 1);
			//}
		}
	}

	CRigidBody* rigidBody = static_cast<CRigidBody*>(GetComponent(COMPONENT_TYPE::RIGIDBODY));
	rigidBody->ReturnPrevLocation(rigidBody->GetVelocity());
}

void CPlayer::OnCollisionExit(CObject* collidedObject)
{
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::STRUCTURE:
		isMove = true;
		break;
	case (UINT)GROUP_TYPE::NPC:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "Pick")
				m_UI[i]->SetActive(false);
		}
		break;
	}
}

void CPlayer::SetUI(CUI* ui)
{
	if (&ui != nullptr)m_UI.push_back(ui);
	cout << ui->GetName() << endl;

	if (ui->GetName() == "Pick")
		ui->SetActive(false);
	else if (ui->GetName() == "1")
		ui->SetActive(false);
	else if (ui->GetName() == "2")
		ui->SetActive(false);
	else if (ui->GetName() == "3")
		ui->SetActive(false);
	else if (ui->GetName() == "4")
		ui->SetActive(false);
	else if (ui->GetName() == "5")
		ui->SetActive(false);
	else if (ui->GetName() == "6")
		ui->SetActive(false);
	else if (ui->GetName() == "7")
		ui->SetActive(false);
	else if (ui->GetName() == "8")
		ui->SetActive(false);
	else if (ui->GetName() == "9")
		ui->SetActive(false);
	else if (ui->GetName() == "firewood")
		ui->SetActive(false);


}

void CPlayer::SetNumber_of_items_UI(const string& ItemName, int prevItemsNum, int NextItemsNum)
{
	switch (prevItemsNum)
	{
	case 0:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "0")
			{
				m_UI[i]->SetActive(false);
			}
			cout << m_UI[i]->GetName() << endl;
		}
		break;
	case 1:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "1")
			{
				cout << "UI : 1" << endl;
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 2:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "2")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 3:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "3")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 4:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "4")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 5:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "5")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 6:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "6")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 7:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "7")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 8:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "8")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	case 9:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "9")
			{
				m_UI[i]->SetActive(false);
				break;
			}
		}
		break;
	}
	// cout << "prevItemsNum : " << prevItemsNum << endl;

	switch (NextItemsNum)
	{
	case 0:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "0")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 1:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "1")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 2:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "2")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 3:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "3")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 4:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "4")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 5:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "5")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 6:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "6")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 7:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "7")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 8:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "8")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	case 9:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "9")
			{
				m_UI[i]->SetActive(true);
				break;
			}
		}
		break;
	}
	// cout << "NextItemsNum : " << NextItemsNum << endl;

}

void CPlayer::GetItem(string item)
{
	for (int i = 0; i < 18; ++i) {
		if (myItems[i].empty()) {
			myItems[i] = item;
			return;
		}
	}
}

CInventory::CInventory()
{
}

CInventory::~CInventory()
{
}
bool CInventory::exchangeItem(const string& itemName, int quantity, const string& newItemName, int newQuantity)
{
	// ������ �˻�
	auto it = find_if(items.begin(), items.end(), [&](const pair<string, int>& item) {
		return item.first == itemName;
		});

	// �������� �������� �ʰų� ��û�� �������� ���� ���?��ȯ ����
	if (it == items.end() || it->second < quantity) {
		// cout << "��ȯ ����" << endl;
		displayInventory();
		return false;
	}

	deleteItem(itemName, quantity);

	addItem(newItemName, newQuantity);

	// cout << "��ȯ ����" << endl;
	displayInventory();
	return true;
}