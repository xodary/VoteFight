#include "pch.h"
#include "Player.h"

#include "GameFramework.h"

#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"

#include "Scene.h"

#include "Camera.h"

#include "StateMachine.h"
#include "Transform.h"

#include "PlayerStates.h"

CPlayer::CPlayer() :
	m_isAiming(),
	m_bulletCount()
{
	SetName("Player");
}

CPlayer::~CPlayer()
{
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
	// 시작 시, 들고있는 권총을 비활성화 시킨다.
	CObject* weapon = FindFrame("gun_pr_1");

	weapon->SetActive(false);
	SetWeapon(weapon);
	m_bulletCount = 5;

	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CPlayerIdleState::GetInstance());
}

void CPlayer::SwapWeapon(WEAPON_TYPE weaponType)
{
	CObject* weapon = GetWeapon();

	if (weapon != nullptr)
	{
		CCharacter::SwapWeapon(weaponType);

		const vector<CObject*>& uis = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::UI);

		switch (weaponType)
		{
		case WEAPON_TYPE::PUNCH:
			uis[0]->FindFrame("Punch")->SetActive(true);
			uis[0]->FindFrame("Pistol")->SetActive(false);
			break;
		case WEAPON_TYPE::PISTOL:
			uis[0]->FindFrame("Punch")->SetActive(false);
			uis[0]->FindFrame("Pistol")->SetActive(true);
			break;
		}
	}
}

void CPlayer::Punch()
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	const vector<CObject*>& guards = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::ENEMY);

	for (const auto& object : guards)
	{
		CGuard* guard = static_cast<CGuard*>(object);
		CTransform* guardTransform = static_cast<CTransform*>(guard->GetComponent(COMPONENT_TYPE::TRANSFORM));

		if (guard->GetHealth() > 0)
		{
			XMFLOAT3 toGuard = Vector3::Subtract(guardTransform->GetPosition(), transform->GetPosition());
			float dist = Vector3::Length(toGuard);

			if (dist <= 3.0f)
			{
				toGuard = Vector3::Normalize(toGuard);

				XMFLOAT3 forward = transform->GetForward();
				float angle = Vector3::Angle(forward, toGuard);

				// forward 벡터와 toGaurd 벡터가 이루는 각이 80도 이하일 때 공격한다.
				if (angle <= 80.0f)
				{
					// 플레이어의 forward 벡터와 교도관의 forward 벡터 간의 각이 40도 이하일 때 후면 타격(즉사)으로 처리한다.
					angle = Vector3::Angle(forward, guardTransform->GetForward());

					CStateMachine* stateMachine = static_cast<CStateMachine*>(guard->GetComponent(COMPONENT_TYPE::STATE_MACHINE));

					if (angle <= 40.0f)
					{
						guard->SetHealth(0);
						stateMachine->ChangeState(CGuardDieState::GetInstance());
					}
					else
					{
						guard->SetHealth(guard->GetHealth() - 40);
						guard->SetTarget(this);
						stateMachine->ChangeState(CGuardHitState::GetInstance());
					}
				}
			}
		}
	}
}

void CPlayer::Shoot()
{
	CObject* nearestIntersectedRootObject = nullptr;
	CObject* nearestIntersectedObject = nullptr;
	float nearestHitDist = FLT_MAX;

	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
	CTransform* cameraTransform = static_cast<CTransform*>(camera->GetComponent(COMPONENT_TYPE::TRANSFORM));
	const XMFLOAT3& rayOrigin = cameraTransform->GetPosition();
	const XMFLOAT3& rayDirection = cameraTransform->GetForward();

	for (int i = (int)GROUP_TYPE::STRUCTURE; i <= (int)GROUP_TYPE::ENEMY; ++i)
	{
		const vector<CObject*>& objects = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject((GROUP_TYPE)i);

		for (const auto& object : objects)
		{
			if (object->IsActive())
			{
				if (i == (int)GROUP_TYPE::ENEMY)
				{
					CGuard* guard = static_cast<CGuard*>(object);

					if (guard->GetHealth() <= 0)
					{
						continue;
					}
				}

				float hitDist = 0.0f, maxDist = 400.0f;
				CObject* intersectedObject = object->CheckRayIntersection(rayOrigin, rayDirection, hitDist, maxDist);

				if ((intersectedObject != nullptr) && (hitDist < nearestHitDist))
				{
					nearestIntersectedRootObject = object;
					nearestIntersectedObject = intersectedObject;
					nearestHitDist = hitDist;
				}
			}
		}

		if ((nearestIntersectedRootObject != nullptr) && (typeid(*nearestIntersectedRootObject) == typeid(CGuard)))
		{
			CGuard* guard = static_cast<CGuard*>(nearestIntersectedRootObject);
			CStateMachine* stateMachine = static_cast<CStateMachine*>(guard->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
			const string& hitFrameName = nearestIntersectedObject->GetName();

			// 머리에 맞은 경우, 즉사시킨다.
			if (hitFrameName == "hat" || hitFrameName == "head_1" || hitFrameName == "head_2")
			{
				guard->SetHealth(0);
				stateMachine->ChangeState(CGuardDieState::GetInstance());
			}
			else
			{
				guard->SetHealth(guard->GetHealth() - 70);
				guard->SetTarget(this);
				stateMachine->ChangeState(CGuardHitState::GetInstance());
			}
		}
	}

	// 총알 UI를 한개씩 지운다.
	const vector<CObject*>& uis = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::UI);
	const vector<CObject*>& children = uis[0]->FindFrame("Pistol")->GetChildren();

	children[--m_bulletCount]->SetActive(false);
}

void CPlayer::Update()
{
	// 현재 윈도우가 포커싱 상태인지 알아낸다.
	HWND hWnd = GetFocus();

	if (hWnd != nullptr)
	{
		// 윈도우 영역 계산
		RECT rect = {};

		GetWindowRect(hWnd, &rect);

		// 마우스 커서 위치 계산
		POINT oldCursor = { static_cast<LONG>(rect.right / 2), static_cast<LONG>(rect.bottom / 2) };
		POINT cursor = {};

		// 이 함수는 윈도우 전체 영역을 기준으로 커서의 위치를 계산한다.
		GetCursorPos(&cursor);

		XMFLOAT2 delta = {};

		delta.x = (cursor.x - oldCursor.x) * 20.0f * DT;
		delta.y = (cursor.y - oldCursor.y) * 20.0f * DT;

		SetCursorPos(oldCursor.x, oldCursor.y);

		// 카메라 X축 회전
		if (!Math::IsZero(delta.y))
		{
			CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
			CTransform* cameraTransform = static_cast<CTransform*>(camera->GetComponent(COMPONENT_TYPE::TRANSFORM));
			XMFLOAT3 cameraRotation = cameraTransform->GetRotation();

			cameraRotation.x += delta.y;

			if (cameraRotation.x < -5.0f)
			{
				cameraRotation.x -= (cameraRotation.x + 5.0f);
			}
			else if (cameraRotation.x > 5.0f)
			{
				cameraRotation.x -= (cameraRotation.x - 5.0f);
			}

			cameraTransform->SetRotation(cameraRotation);
		}

		// 플레이어 Y축 회전
		if (!Math::IsZero(delta.x))
		{
			CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
			XMFLOAT3 rotation = transform->GetRotation();

			rotation.y += delta.x;

			if (rotation.y < 0.0f)
			{
				rotation.y += 360.0f;
			}
			else if (rotation.y > 360.0f)
			{
				rotation.y -= 360.0f;
			}

			transform->SetRotation(rotation);
		}

		if (KEY_TAP(KEY::NUM1))
		{
			if (IsEquippedWeapon())
			{
				SwapWeapon(WEAPON_TYPE::PUNCH);
			}
		}

		if (KEY_TAP(KEY::NUM2))
		{
			if (!IsEquippedWeapon())
			{
				SwapWeapon(WEAPON_TYPE::PISTOL);
			}
		}
	}

	CObject::Update();
}
