#include "pch.h"
#include "Player.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "Camera.h"
#include "StateMachine.h"
#include "OtherPlayerStates.h"
#include "Transform.h"
#include "PlayerStates.h"
#include "NPC.h"
#include "UI.h"
#include "Bullet.h"
#include "Transform.h"

CPlayer::CPlayer() :
	m_isAiming(),
	m_bulletCount(),
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("Player");
	m_Inventory = new CInventory();
}

CPlayer::~CPlayer()
{
}

void CPlayer::SetClickAngle(float clickAngle)
{
	m_clickAngle = clickAngle;
}

float CPlayer::GetClickAngle()
{
	return m_clickAngle;
}

void CPlayer::SetTurnAngle(float look)
{
	m_turnAngle = look;
}

float CPlayer::GetTurnAngle()
{
	return m_turnAngle;
}

string CPlayer::GetSpineName()
{
	return m_spineName;
}

float CPlayer::GetSpineAngle()
{
	return m_spineAngle;
}

void CPlayer::SetSpineAngle(float angle)
{
	m_spineAngle = angle;
}

void CPlayer::SetAiming(bool isAiming)
{
	m_isAiming = isAiming;
}

bool CPlayer::IsAiming()
{
	return m_isAiming;
}

bool CPlayer::HasBullet()
{
	return m_bulletCount > 0;
}

void CPlayer::Init()
{
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CPlayerIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->SetWeight("idle", 1.0f);
	m_Inventory = new CInventory();

	
}

void CPlayer::AnotherInit()
{
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(COtherPlayerIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->SetWeight("idle", 1.0f);
	m_Inventory = new CInventory();

}

void CPlayer::SwapWeapon(WEAPON_TYPE weaponType)
{
}

void CPlayer::Punch()
{
}

void CPlayer::Shoot(CScene& currScene)
{
	if (currScene.GetName() == "GameScene")
	{
		currScene.object_manager.ShowObjects();
		CBullet* Bullet = static_cast<CBullet*>(Load("Bullet"));
		if (Bullet) {
			XMFLOAT3 currPostion(GetPostion());
			currPostion.y += 1.f;
			Bullet->SetPostion(currPostion);
			Bullet->SetRotate(GetRotate());
			Bullet->SetScale(XMFLOAT3(5, 5, 5));
			currScene.AddObject(GROUP_TYPE::BULLET, Bullet);
			cout << "발사됨" << endl;
		}
	}
}

void CPlayer::Update()
{
	CObject::Update();
	
}

void CPlayer::OnCollisionEnter(CObject* collidedObject)
{
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::STRUCTURE:
		isMove = false;
	case (UINT)GROUP_TYPE::NPC:
		for (size_t i = 0; i < m_UI.size(); i++)
		{
			if (m_UI[i]->GetName() == "Pick")
				m_UI[i]->SetActive(true);
		}
		break;
	}
}
void CPlayer::OnCollision(CObject* collidedObject)
{
	if (KEY_TAP(KEY::F))
	{
		if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::STRUCTURE)
		{
			m_Inventory->addItem(collidedObject->GetName(), 1);
			collidedObject->SetDeleted(true);
		}
		if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::NPC)
		{
			CNPC* targetNPC = (CNPC*)collidedObject;
			if (!targetNPC->GetQuest()->getCompletionStatus())
			{
				SetNumber_of_items_UI("tiket", m_Inventory->getItems("tiket"), m_Inventory->getItems("tiket") + 1);
				m_Inventory->exchangeItem(targetNPC->GetQuest()->GetItemName(), targetNPC->GetQuest()->GetItemQuantity(), "tiket", 1);
			}
		}
	}
}
void CPlayer::OnCollisionExit(CObject* collidedObject)
{
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::STRUCTURE:
		isMove = true;
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


//
/// 아이템 및 인벤토리 

CInventory::CInventory()
{
}

CInventory::~CInventory()
{
}
bool CInventory::exchangeItem(const string& itemName, int quantity, const string& newItemName, int newQuantity)
{
	// 아이템 검색
	auto it = find_if(items.begin(), items.end(), [&](const pair<string, int>& item) {
		return item.first == itemName;
		});

	// 아이템이 존재하지 않거나 요청된 수량보다 적을 경우 교환 실패
	if (it == items.end() || it->second < quantity) {
		// cout << "교환 실패" << endl;
		displayInventory();
		return false;
	}

	deleteItem(itemName, quantity);

	addItem(newItemName, newQuantity);

	// cout << "교환 성공" << endl;
	displayInventory();
	return true;
}