//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext)
{
	m_pCamera = CreateCamera(0.0f);	

	CLoadedModelInfo* pSimpsonModel = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/hugo_idle.bin", NULL);
	SetChild(pSimpsonModel->m_pModelRootObject, true);

	m_pSkinnedAnimationController = new CPlayerAnimationController(pd3dDevice, pd3dCommandList, 5, pSimpsonModel);
	m_pSkinnedAnimationController->m_pRootMotionObject = pSimpsonModel->m_pModelRootObject->FindFrame("mixamorig:Hips");

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);	// idle

	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);	// turnRight
	m_pSkinnedAnimationController->SetTrackSpeed(1, TURNSPEED);

	m_pSkinnedAnimationController->SetTrackAnimationSet(2, 2);	// turnLeft
	m_pSkinnedAnimationController->SetTrackSpeed(2, TURNSPEED);

	m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);	// Walk

	// m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);	// Dance

	m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);	// idle (upper-body)


	// �ִϸ��̼� ����ü �и�
	m_pSkinnedAnimationController->CreateMaskBones(0, 1, false);	
	m_pSkinnedAnimationController->CreateMaskBones(1, 1, false);	// Spine�� ������
	m_pSkinnedAnimationController->CreateMaskBones(2, 1, false);	// Spine�� ������
	m_pSkinnedAnimationController->CreateMaskBones(3, 1, false);	// Spine�� ������
	m_pSkinnedAnimationController->CreateMaskBones(4, 1, true);	// Spine�� �츮�� �������� ������.
	m_pSkinnedAnimationController->SetMaskBone(0, 0, "mixamorig:Spine");
	m_pSkinnedAnimationController->SetMaskBone(1, 0, "mixamorig:Spine");
	m_pSkinnedAnimationController->SetMaskBone(2, 0, "mixamorig:Spine");
	m_pSkinnedAnimationController->SetMaskBone(3, 0, "mixamorig:Spine");
	m_pSkinnedAnimationController->SetMaskBone(4, 0, "mixamorig:Spine");

#ifdef UPPER_BODY
	// �ִϸ��̼� ����ü �и�
	m_pSkinnedAnimationController->CreateMaskBones(3, 1, false);	// Spine�� ������
	m_pSkinnedAnimationController->CreateMaskBones(4, 1, true);
	m_pSkinnedAnimationController->SetMaskBone(3, 0, "mixamorig:Spine");
	m_pSkinnedAnimationController->SetMaskBone(4, 0, "mixamorig:Spine");
#endif

	SetRootMotion(true);
#ifdef UPPER_BODY
	m_pSkinnedAnimationController->SetTrackWeight(0, 0);
	m_pSkinnedAnimationController->SetTrackWeight(1, 0);
	m_pSkinnedAnimationController->SetTrackWeight(2, 0);
	m_pSkinnedAnimationController->SetTrackWeight(3, 1);
	m_pSkinnedAnimationController->SetTrackWeight(4, 1);

	m_pSkinnedAnimationController->SetTrackEnable(0, false);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, true);
	m_pSkinnedAnimationController->SetTrackEnable(4, true);

#else
	m_pSkinnedAnimationController->SetTrackWeight(0, 1);
	m_pSkinnedAnimationController->SetTrackWeight(1, 0);
	m_pSkinnedAnimationController->SetTrackWeight(2, 0);
	m_pSkinnedAnimationController->SetTrackWeight(3, 0);
	m_pSkinnedAnimationController->SetTrackWeight(4, 1);

	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, false);
	m_pSkinnedAnimationController->SetTrackEnable(2, false);
	m_pSkinnedAnimationController->SetTrackEnable(3, false);
	m_pSkinnedAnimationController->SetTrackEnable(4, true);
#endif

	m_pSkinnedAnimationController->m_pAnimationTracks[1].m_nType = ANIMATION_TYPE_ONCE;
	m_pSkinnedAnimationController->m_pAnimationTracks[2].m_nType = ANIMATION_TYPE_ONCE;



	m_pSkinnedAnimationController->SetCallbackKeys(1, 2);
#ifdef _WITH_SOUND_RESOURCE
	m_pSkinnedAnimationController->SetCallbackKey(0, 0.1f, _T("Footstep01"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 0.5f, _T("Footstep02"));
	m_pSkinnedAnimationController->SetCallbackKey(2, 0.9f, _T("Footstep03"));
#else
	m_pSkinnedAnimationController->SetCallbackKey(1, 0, 0.2f, _T("Sound/Footstep01.wav"));
	m_pSkinnedAnimationController->SetCallbackKey(1, 1, 0.5f, _T("Sound/Footstep02.wav"));
	//	m_pSkinnedAnimationController->SetCallbackKey(1, 2, 0.39f, _T("Sound/Footstep03.wav"));
#endif
	CAnimationCallbackHandler* pAnimationCallbackHandler = new CSoundCallbackHandler();
	m_pSkinnedAnimationController->SetAnimationCallbackHandler(1, pAnimationCallbackHandler);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	// CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

	m_pCrntState = new playerState::Idle(this);

	if (pSimpsonModel) delete pSimpsonModel;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pcbMappedGameObject)
	{
		XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
		if (m_pChild->m_pChild->m_ppMaterials)
		{
			CMaterial* pMaterial = m_pChild->m_pChild->m_ppMaterials[0];
			XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&pMaterial->m_ppTextures[0]->m_xmf4x4Texture)));
			MATERIAL material = { pMaterial->m_xmf4AlbedoColor, pMaterial->m_xmf4EmissiveColor, pMaterial->m_xmf4SpecularColor, pMaterial->m_xmf4AmbientColor };
			memcpy(&m_pcbMappedGameObject->m_material, &material, sizeof(MATERIAL));
			memcpy(&m_pcbMappedGameObject->m_nType, &pMaterial->m_nType, sizeof(UINT));
		}
	}

	// CGameObject::UpdateShaderVariables(pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);
}

void CPlayer::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	/*if (m_pCrntState)
	{
		m_pCrntState->Update(fTimeElapsed);
		BlendAnimation(fTimeElapsed);
	}*/
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	UpdateTransform(NULL);
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	CGameObject::Render(pd3dCommandList, pCamera);
}

CCamera* CPlayer::CreateCamera(float fTimeElapsed)
{
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetMaxVelocityXZ(15.0f);
	SetMaxVelocityY(400.0f);
	m_pCamera = new CCamera(m_pCamera);
	m_pCamera->SetPlayer(this);
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 80.0f, -80.0f));
	m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return(m_pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
#define _WITH_DEBUG_CALLBACK_DATA

void CSoundCallbackHandler::HandleCallback(void *pCallbackData, float fTrackPosition)
{
   _TCHAR *pWavName = (_TCHAR *)pCallbackData; 
#ifdef _WITH_DEBUG_CALLBACK_DATA
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("%s(%f)\n"), pWavName, fTrackPosition);
	OutputDebugString(pstrDebug);
#endif
#ifdef _WITH_SOUND_RESOURCE
   PlaySound(pWavName, ::ghAppInstance, SND_RESOURCE | SND_ASYNC);
#else
   //PlaySound(pWavName, NULL, SND_FILENAME | SND_ASYNC);
#endif
}



void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	// float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 10.0f;
	// if (xmf3PlayerPosition.y < fHeight)
	// {
	// 	XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
	// 	xmf3PlayerVelocity.y = 0.0f;
	// 	SetVelocity(xmf3PlayerVelocity);
	// 	xmf3PlayerPosition.y = fHeight;
	// 	SetPosition(xmf3PlayerPosition);
	// }
}

void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	// float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	// if (xmf3CameraPosition.y <= fHeight)
	// {
	// 	xmf3CameraPosition.y = fHeight;
	// 	m_pCamera->SetPosition(xmf3CameraPosition);
	// 	CCamera* p3rdPersonCamera = m_pCamera;
	// 	p3rdPersonCamera->SetLookAt(GetPosition());
	// }
}

void CPlayer::Move(DWORD xmf3Direction, float fDistance, bool bUpdateVelocity)
{
	if (xmf3Direction)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (xmf3Direction & DIR_FORWARD) xmf3Shift.z += 1;
		if (xmf3Direction & DIR_BACKWARD) xmf3Shift.z -= 1;		
		if (xmf3Direction & DIR_RIGHT) xmf3Shift.x += 1;
		if (xmf3Direction & DIR_LEFT) xmf3Shift.x -= 1;

		xmf3Shift = Vector3::Normalize(xmf3Shift);
		SetLookEnd(xmf3Shift);
		xmf3Shift = Vector3::Add(xmf3Shift, xmf3Shift, fDistance);
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Update(float fTimeElapsed)
{
	// Look Up Right ���� Normalize
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Up = Vector3::Normalize(m_xmf3Up);
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true));
	
	// �߷�, �̵� �ӷ� ���
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	// Terrain���� Player�� Camera ���
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	// ������
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	// ���� ���¿� ���� Update �Լ� ȣ�� �� �ִϸ��̼� ����
	if (m_pCrntState)
	{
		m_pCrntState->Update(fTimeElapsed);
		BlendAnimation(fTimeElapsed);
	}
}

using namespace playerState;

// Player�� Animation State�� ����� �� ȣ���ϴ� �Լ�
// Args:
//	playerState: �����ϰ��� �ϴ� ����
//	e: �߻��� �̺�Ʈ�� ����
//	xmf3Direction: �����ϰ��� �ϴ� ���� ����
// 2024-02-21 13:43 Ȳ���� ����
void CPlayer::ChangeState(const playerState::State& playerState, const Event& e, const XMFLOAT3& xmf3Direction)
{
	// Exit �Լ� ȣ��
	if (m_pCrntState != nullptr)
	{
		m_pCrntState->Exit(playerState);
		delete m_pCrntState;
	}

	// ���� ��ü ����
	switch (playerState)
	{
	case State::Idle:
		m_pCrntState = new playerState::Idle(this);
		break;
	case State::Walk:
		m_pCrntState = new playerState::Walk(this);
		break;
	case State::TurnRight:
		m_pCrntState = new playerState::TurnRight(this);
		break;
	case State::TurnLeft:
		m_pCrntState = new playerState::TurnLeft(this);
		break;
	default:
		assert(0);
	}

	// Enter �Լ� ȣ��
	m_pCrntState->Enter(e, xmf3Direction);
}

// Player�� Animation�� �ڿ������� �����ϴ� �Լ�
// Args:
//	fTimeElapsed: ���� Ŭ���� ���� Ŭ�� ������ �ð�
// 2024-02-21 13:43 Ȳ���� ����
void CPlayer::BlendAnimation(float fTimeElapsed)
{
	if (m_bBlending)
	{
		// m_nUpState�� ������ �������� �ִ� �ִϸ��̼��� �ε���, m_nDownState�� ������ �������� �ִ� �ִϸ��̼��� �ε����̴�.
		// m_fUpState�� ������ �������� �ִ� �ִϸ��̼��� ���� ����, m_fDownState�� ������ �������� �ִ� �ִϸ��̼��� ���� �����̴�.
		// �Լ��� ȣ��� ���� m_fUpState�� 0���� 1�� ���ϰ�, m_fDownState�� 1���� 0���� ����.
		m_fUpState = min(1.f, m_fUpState + 5.f * fTimeElapsed);
		m_fDownState = 1.0f - m_fUpState;

		// ������ ������ �������� m_fDownState�� �ش��ϴ� �ִϸ��̼��� Enable ��Ų��.
		if (m_fDownState <= 0.0f)
		{
			m_pSkinnedAnimationController->SetTrackEnable(m_nDownState, false);
			m_fDownState = 0.0f;
			m_fUpState = 1.0f;
			m_pSkinnedAnimationController->SetTrackWeight(m_nUpState, m_fUpState);
			m_pSkinnedAnimationController->SetTrackWeight(m_nDownState, m_fDownState);
			m_bBlending = false;
		}

		m_pSkinnedAnimationController->SetTrackWeight(m_nUpState, m_fUpState);
		m_pSkinnedAnimationController->SetTrackWeight(m_nDownState, m_fDownState);
	}
}


/********** [ IDLE ] **********/

// Idle ������ ���� ó�� ���� �� ȣ��
// Args:
//	e: �Էµ� �̺�Ʈ�� ����
//	xmf3Direction: �Էµ� ������ ����
// 2024-02-21 21:45 Ȳ���� ����
void playerState::Idle::Enter(const Event& e, const XMFLOAT3& xmf3Direction)
{
	// IDLE�� �ش��ϴ� �ε����� �ִϸ��̼��� Ȱ��ȭ �Ѵ�.
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(IDLE, true);
}

// Idle ������ ���� ���� �� ȣ��
// Args:
//	playerState: ���� ����
// 2024-02-21 21:45 Ȳ���� ����
void playerState::Idle::Exit(const State& playerState)
{
	// �ִϸ��̼� ������ ���� ���� ����
	// CPlayer::BlendAnimation ���� �� ������ �����.
	m_pPlayer->m_nUpState = int(playerState);
	m_pPlayer->m_nDownState = IDLE;

	for (int i = 0; i < 4; ++i)
	{
		if (i == m_pPlayer->m_nUpState || i == m_pPlayer->m_nDownState)
			continue;
		m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, 0);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, ANIMATION_CALLBACK_EPSILON);
	}
	m_pPlayer->m_fUpState = 0.0f;
	m_pPlayer->m_bBlending = true;
}

// Idle ������ �� ��� ȣ��
// Args:
//	fTimeElapsed: ���� Ŭ���� ���� Ŭ�� ������ �ð�
// 2024-02-21 21:45 Ȳ���� ����
void playerState::Idle::Update(float fTimeElapsed)
{
}

// Idle ������ �� �ٸ� �̺�Ʈ�� �߻��ߴٸ�, ó���� ���ִ� �Լ�
// Args:
//	e: �Էµ� �̺�Ʈ�� ����
//	xmf3Direction: �Էµ� ������ ����
// 2024-02-21 13:43 Ȳ���� ����
void playerState::Idle::HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction)
{
	XMFLOAT3 xmf3SubDirection = xmf3Direction;
	float dotProduct = Vector3::DotProduct(xmf3SubDirection, m_pPlayer->m_xmf3Look);
	float angleInDegrees = XMConvertToDegrees(acos(dotProduct));
	XMFLOAT3 crossProduct = Vector3::CrossProduct(xmf3SubDirection, m_pPlayer->m_xmf3Look);
	if (crossProduct.y > 0)
		angleInDegrees = -angleInDegrees;
	// std::cout << "���� (��): " << angleInDegrees << std::endl;

	// �Էµ� ���Ͱ� ���� ������ ���̰��� -45�� ���� �۰ų� 45�� ���� ũ�ٸ� Turn Animation�� �����Ų��.
	if (angleInDegrees >= 45)
	{
		m_pPlayer->ChangeState(State::TurnRight, e, xmf3SubDirection);
	}
	else if (angleInDegrees <= -45)
	{
		m_pPlayer->ChangeState(State::TurnLeft, e, xmf3SubDirection);
	}
	else // �ƴ϶�� Walk Animation�� �����Ų��.
	{
		if(e == Event::KeyDown)
			m_pPlayer->ChangeState(State::Walk, e, xmf3SubDirection);
	}
}

/********** [ WALK ] **********/

void playerState::Walk::Enter(const Event& e, const XMFLOAT3& xmf3Direction)
{
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(TURNRIGHT, false);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(TURNLEFT, false);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(WALK, true);
	m_xmf3Direction = xmf3Direction;
}

void playerState::Walk::Exit(const State& playerState)
{
	for (int i = 0; i < 4; ++i)
	{
		if (i == m_pPlayer->m_nUpState || i == m_pPlayer->m_nDownState)
			continue;
		m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, 0);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, ANIMATION_CALLBACK_EPSILON);
	}
	m_pPlayer->m_nUpState = int(playerState);
	m_pPlayer->m_nDownState = WALK;
	m_pPlayer->m_fUpState = 0.0f;
	m_pPlayer->m_bBlending = true;
}

void playerState::Walk::Update(float fTimeElapsed)
{
	m_pPlayer->Move(Vector3::ScalarProduct(m_xmf3Direction, fTimeElapsed * 1000), true);
}

void playerState::Walk::HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction)
{
	switch (e)
	{
	case Event::KeyDown:

		if (m_pPlayer->m_xmf3Velocity.x == 0 && m_pPlayer->m_xmf3Velocity.z == 0)
			m_pPlayer->ChangeState(State::Idle, e, xmf3Direction);
		else
		{
			m_xmf3Direction = xmf3Direction;
			m_pPlayer->m_xmf3Look = Vector3::Interpolation(m_pPlayer->m_xmf3Look, m_xmf3Direction, 0.1);
			m_pPlayer->m_xmf3Look = Vector3::Normalize(m_pPlayer->m_xmf3Look);
		}

		/*
		if (m_pPlayer->GetDirX() == 0 && m_pPlayer->GetDirZ() == 0)
		{
			m_pPlayer->ChangeState(CPlayer::State::Idle, e, key);
		}
		else if (key == GLUT_KEY_SHIFT_L)
		{
			m_pPlayer->Run();
		}*/
		break;
	case Event::KeyUp:
		//if (movKeys.find(key) != movKeys.end())
		//{
		//	m_pPlayer->SubDir(key);
		//}

		m_pPlayer->ChangeState(State::Idle, e, xmf3Direction);

		//else if (key == GLUT_KEY_SHIFT_L)
		//{
		//	m_pPlayer->StopRun();
		//}
		break;
	}
}

void playerState::TurnRight::Enter(const Event& e, const XMFLOAT3& xmf3Direction)
{
	m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNRIGHT].m_bAnimationOnceDone = false;
	m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(TURNRIGHT, ANIMATION_CALLBACK_EPSILON);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(TURNRIGHT, true);
	m_xmf3Direction = xmf3Direction;
	m_xmf3Origin = m_pPlayer->m_xmf3Look;
	float dotProduct = Vector3::DotProduct(m_xmf3Direction, m_xmf3Origin);
	float angleInDegrees = XMConvertToDegrees(acos(dotProduct));
	m_nTurn = (int)(angleInDegrees / 45);
}

// TurnRight ������ �� ��� ȣ���.
// Args:
//	fTimeElapsed: ���� Ŭ���� ���� Ŭ�� ������ �ð�
// 2024-02-21 13:43 Ȳ���� ����
void playerState::TurnRight::Exit(const State& playerState)
{
	for (int i = 0; i < 4; ++i)
	{
		if (i == m_pPlayer->m_nUpState || i == m_pPlayer->m_nDownState)
			continue;
		m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, 0);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, ANIMATION_CALLBACK_EPSILON);
	}
	m_pPlayer->m_nUpState = int(playerState);
	m_pPlayer->m_nDownState = TURNRIGHT;
	m_pPlayer->m_fUpState = 0.0f;
	m_pPlayer->m_bBlending = true;
}

// TurnRight ������ �� ��� ȣ���.
// Args:
//	fTimeElapsed: ���� Ŭ���� ���� Ŭ�� ������ �ð�
// 2024-02-21 13:43 Ȳ���� ����
void playerState::TurnRight::Update(float fTimeElapsed)
{
	// ���ϴ� ������ ���ݾ� ȸ����Ű�� ����.
	float time = m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[TURNLEFT]->m_fLength / TURNSPEED;
	float fRotateAngle = m_nTurn * 45 / time * fTimeElapsed;	
	m_pPlayer->Rotate(0, fRotateAngle, 0);

	// Turn �ִϸ��̼��� ������ Idle ���·� ���ư���.
	if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNRIGHT].m_bAnimationOnceDone)
	{
		m_pPlayer->m_xmf3Look = Vector3::TransformCoord(m_xmf3Origin, Matrix4x4::Rotate(0, m_nTurn * 45, 0));
		m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNRIGHT].m_bAnimationOnceDone = false;
		m_pPlayer->ChangeState(State::Idle, Event::TurnDone, m_pPlayer->m_xmf3Look);
	}
}

void playerState::TurnRight::HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction)
{
}

// TurnLeft�� TurnRight�� ���� �����ϴ�. �ִϸ��̼� �ε����� ȸ�� ���⸸ �ٸ���.
void playerState::TurnLeft::Enter(const Event& e, const XMFLOAT3& xmf3Direction)
{
	m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNLEFT].m_bAnimationOnceDone = false;
		m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(TURNLEFT, ANIMATION_CALLBACK_EPSILON);
	m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(TURNLEFT, true);
	m_xmf3Direction = xmf3Direction;
	m_xmf3Origin = m_pPlayer->m_xmf3Look;
	float dotProduct = Vector3::DotProduct(m_xmf3Direction, m_xmf3Origin);
	float angleInDegrees = XMConvertToDegrees(acos(dotProduct));
	m_nTurn = (int)(angleInDegrees / 45);
}

void playerState::TurnLeft::Exit(const State& playerState)
{
	for (int i = 0; i < 4; ++i)
	{
		if (i == m_pPlayer->m_nUpState || i == m_pPlayer->m_nDownState)
			continue;
		m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, 0);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, false);
		m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, ANIMATION_CALLBACK_EPSILON);
	}
	m_pPlayer->m_nUpState = int(playerState);
	m_pPlayer->m_nDownState = TURNLEFT;
	m_pPlayer->m_fUpState = 0.0f;
	m_pPlayer->m_bBlending = true;
}

void playerState::TurnLeft::Update(float fTimeElapsed)
{
	float time = m_pPlayer->m_pSkinnedAnimationController->m_pAnimationSets->m_pAnimationSets[TURNLEFT]->m_fLength / TURNSPEED;
	float fRotateAngle = m_nTurn * 45 / time * fTimeElapsed;
	m_pPlayer->Rotate(0, -fRotateAngle, 0);

	if (m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNLEFT].m_bAnimationOnceDone)
	{
		m_pPlayer->m_xmf3Look = Vector3::TransformCoord(m_xmf3Origin, Matrix4x4::Rotate(0, -m_nTurn * 45, 0));
		m_pPlayer->m_pSkinnedAnimationController->m_pAnimationTracks[TURNLEFT].m_bAnimationOnceDone = false;
		m_pPlayer->ChangeState(State::Idle, Event::TurnDone, m_pPlayer->m_xmf3Look);
	}
}

void playerState::TurnLeft::HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction)
{
}
