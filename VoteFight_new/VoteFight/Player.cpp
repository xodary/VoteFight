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
#include "Transform.h"
#include "PlayerStates.h"
#include "NPC.h"
#include "UI.h"

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

void CPlayer::SwapWeapon(WEAPON_TYPE weaponType)
{
}

void CPlayer::Punch()
{
}

void CPlayer::Shoot()
{
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
//	m_UI->SetActive(false);
}


//
/// ������ �� �κ��丮 

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

	// �������� �������� �ʰų� ��û�� �������� ���� ��� ��ȯ ����
	if (it == items.end() || it->second < quantity) {
		cout << "��ȯ ����" << endl;
		displayInventory();
		return false;
	}

	deleteItem(itemName, quantity);

	addItem(newItemName, newQuantity);

	cout << "��ȯ ����" << endl;
	displayInventory();
	return true;
}