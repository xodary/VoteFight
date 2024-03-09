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
}

void CPlayer::Shoot()
{
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
