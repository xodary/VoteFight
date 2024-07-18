#include "pch.h"
#include "Monster.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "StateMachine.h"
#include "Transform.h"
#include "MonsterStates.h"

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
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CMonsterWalkState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));

	animator->SetWeight("idle", 1.0f);
	animator->Play("idle", true);

}

void CMonster::Update()
{
	CObject::Update();

	if (m_dead) {
		CAnimator* animator = reinterpret_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
		if (animator->IsFinished())		// 죽는 몬스터 애니메이션 끝나면 삭제
		{
			CSceneManager::GetInstance()->GetCurrentScene()->DeleteObject(GROUP_TYPE::MONSTER, m_id);
		}
		
		return;
	}

	//if (AimObejct)
	//{
	//	XMFLOAT3 MyPostion = GetPosition();
	//	XMFLOAT3 TargetPostion = AimObejct->GetPosition();
	//	double dx = TargetPostion.x - MyPostion.x;
	//	double dz = TargetPostion.z - MyPostion.z;
	//	float angle = atan2(dz, dx); // 또는 M_PI

	//	MyPostion.x += m_fSpeed * cos(angle);
	//	MyPostion.z += m_fSpeed * sin(angle);

	//	SetPostion(MyPostion);

	//	XMFLOAT3 direction;
	//	direction.x = 0;
	//	direction.y = -angle * (180.0 / 3.14) + 90;
	//	direction.z = 0;

	//	SetRotate(direction);
	//}
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

