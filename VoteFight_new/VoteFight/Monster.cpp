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

CMonster::CMonster()
{
	SetName("Monster");
	SetGroupType((UINT)GROUP_TYPE::MONSTER);
}

CMonster::~CMonster()
{
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

	if (m_bilboardUI.size() > 0 && (chrono::system_clock::now() - m_bilboardUI[0]->maketime) > 1s)
		m_bilboardUI.clear();
}
