#include "pch.h"
#include "Monster.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "StateMachine.h"
#include "Transform.h"
#include "UI.h"

CMonster::CMonster() :
	m_spineName("mixamorig:Spine"),
	m_spineAngle(),
	m_turnAngle()
{
	SetName("Monster");
	SetGroupType((UINT)GROUP_TYPE::MONSTER);
	
}

CMonster::~CMonster()
{
}


string CMonster::GetSpineName()
{
	return m_spineName;
}

float CMonster::GetSpineAngle()
{
	return m_spineAngle;
}

void CMonster::SetSpineAngle(float angle)
{
	m_spineAngle = angle;
}

void CMonster::Init()
{
	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->SetAnimateBone(FindFrame("mixamorig:Hips"), ANIMATION_BONE::ROOT);
	animator->SetWeight("idle", ANIMATION_BONE::ROOT, 1.0f);
	animator->Play("idle", true);
}

void CMonster::Update()
{
	CObject::Update();

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	if (m_bilboardUI.size() > 0 && (chrono::system_clock::now() - m_bilboardUI[0]->maketime) > 1s)
		m_bilboardUI.clear();
}

void CMonster::PlayerDiscovery(CObject* player)
{
	//// 플레이어 발견 조건
	//AimObejct = player;
}

void CMonster::OnCollisionEnter(CObject* collidedObject)
{
	switch (collidedObject->GetGroupType())
	{
	case (UINT)GROUP_TYPE::BULLET:
		SetHealth(GetHealth()-10);
		cout << "몬스터 히트" << endl;
		break;
	}
}

void CMonster::OnCollisionExit(CObject* collidedObject)
{
}

