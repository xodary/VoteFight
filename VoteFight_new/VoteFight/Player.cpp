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
	SetGroupType((UINT)GROUP_TYPE::PLAYER);
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	CAnimator* animator = reinterpret_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->SetAnimateBone(FindFrame("mixamorig:Hips"), ANIMATION_BONE::LOWER);
	animator->SetAnimateBone(FindFrame("mixamorig:LeftUpLeg"), ANIMATION_BONE::LOWER);
	animator->SetAnimateBone(FindFrame("mixamorig:RightUpLeg"), ANIMATION_BONE::LOWER);
	animator->SetMaskBone(FindFrame("mixamorig:Spine"), ANIMATION_BONE::LOWER);
	animator->SetAnimateBone(FindFrame("mixamorig:Spine"), ANIMATION_BONE::UPPER);
	animator->SetWeight("Idle", ANIMATION_BONE::LOWER, 1.0f);
	animator->SetWeight("Idle", ANIMATION_BONE::UPPER, 1.0f);
	animator->Play("Idle", true);

	//m_bilboardUI.emplace_back(new CHPbarUI(this));
	//m_bilboardUI.push_back(new CTextUI(this));
}

void CPlayer::Attack()
{
	if (CSceneManager::GetInstance()->GetCurrentScene()->GetName() == "GameScene")
	{
		CS_ATTACK_PACKET p;
		p.m_size = sizeof(p);
		p.m_type = P_CS_ATTACK_PACKET;
		p.m_angle = GetRotate().y;
		p.m_pos = FindFrame("GunPos")->GetPosition();
		p.m_pos.y = FindFrame("GunPos")->GetPosition().y - 1.f;

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

void CPlayer::Update()
{
	CCharacter::Update();
	
	if (m_bilboardUI.size() > 0 && (chrono::system_clock::now() - m_bilboardUI[0]->maketime) > 1s)
		m_bilboardUI.clear();
}

void CPlayer::OnCollisionEnter(CObject* collidedObject)
{
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
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::STRUCTURE:
	case (UINT)GROUP_TYPE::BOX:
		CRigidBody* rigidBody = static_cast<CRigidBody*>(GetComponent(COMPONENT_TYPE::RIGIDBODY));
		rigidBody->ReturnPrevLocation(rigidBody->GetVelocity());
		break;
	}
}

void CPlayer::OnCollisionExit(CObject* collidedObject)
{
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