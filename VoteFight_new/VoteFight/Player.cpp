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
	// ���� �����찡 ��Ŀ�� �������� �˾Ƴ���.
	HWND hWnd = GetFocus();

	if (hWnd != nullptr)
	{
		// ������ ���� ���
		RECT rect = {};

		GetWindowRect(hWnd, &rect);

		// ���콺 Ŀ�� ��ġ ���
		POINT oldCursor = { static_cast<LONG>(rect.right / 2), static_cast<LONG>(rect.bottom / 2) };
		POINT cursor = {};

		// �� �Լ��� ������ ��ü ������ �������� Ŀ���� ��ġ�� ����Ѵ�.
		GetCursorPos(&cursor);

		XMFLOAT2 delta = {};

		delta.x = (cursor.x - oldCursor.x) * 20.0f * DT;
		delta.y = (cursor.y - oldCursor.y) * 20.0f * DT;

		SetCursorPos(oldCursor.x, oldCursor.y);

		// ī�޶� X�� ȸ��
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

		// �÷��̾� Y�� ȸ��
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
