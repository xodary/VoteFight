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

	m_pSkinnedAnimationController = new CPlayerAnimationController(pd3dDevice, pd3dCommandList, 2, pSimpsonModel);
	m_pSkinnedAnimationController->m_pRootMotionObject = pSimpsonModel->m_pModelRootObject->FindFrame("mixamorig:Hips");

	m_pSkinnedAnimationController->SetTrackAnimationSet(0, 0);	// idle
	m_pSkinnedAnimationController->SetTrackAnimationSet(1, 1);	// leg walk

	m_pSkinnedAnimationController->CreateMaskBones(0, 1, true);
	m_pSkinnedAnimationController->CreateMaskBones(1, 1, false);
	
	m_pSkinnedAnimationController->SetMaskBone(0, 0, "mixamorig:Spine");
	
	m_pSkinnedAnimationController->SetMaskBone(1, 0, "mixamorig:Spine");
	SetRootMotion(true);

	//m_pSkinnedAnimationController->SetTrackAnimationSet(3, 3);	// sidewalk_left
	//m_pSkinnedAnimationController->SetTrackAnimationSet(4, 4);	// sidewalk_right 
	//m_pSkinnedAnimationController->SetTrackAnimationSet(5, 5);	// leg walk 

	m_pSkinnedAnimationController->SetTrackWeight(0, 1);
	m_pSkinnedAnimationController->SetTrackWeight(1, 1);

	//m_pSkinnedAnimationController->SetTrackWeight(IDLE, idle);
	//m_pSkinnedAnimationController->SetTrackWeight(WALK, walk);

	//"mixamorig:LeftHandIndex2 mixamorig:LeftHand mixamorig:LeftHandThumb2 mixamorig:LeftHandThumb3 mixamorig:LeftHandIndex3 mixamorig:RightForeArm mixamorig:RightHand mixamorig:RightHandIndex1 mixamorig:RightHandIndex2 mixamorig:RightHandIndex3 mixamorig:RightHandThumb1 mixamorig:RightHandThumb2 mixamorig:RightHandThumb3 mixamorig:Spine1 mixamorig:LeftForeArm"
	m_pSkinnedAnimationController->SetTrackEnable(0, true);
	m_pSkinnedAnimationController->SetTrackEnable(1, true);
	//m_pSkinnedAnimationController->SetTrackEnable(4, false);
	//m_pSkinnedAnimationController->SetTrackEnable(5, false);

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

	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;
	m_xmf3Position = XMFLOAT3(310.0f, pTerrain->GetHeight(310.0f, 590.0f) + 100.f, 590.0f);
	SetScale(XMFLOAT3(10.0f, 10.0f, 10.0f));

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
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
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
	m_pCamera->Rotate(x, y, z);
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

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	UpdateTransform(NULL);
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	CGameObject::Render(pd3dCommandList, pCamera);
}

CCamera* CPlayer::CreateCamera(float fTimeElapsed)
{
	SetFriction(250.0f);
	SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
	SetMaxVelocityXZ(15.0f);
	SetMaxVelocityY(400.0f);
	m_pCamera = new CCamera(m_pCamera);
	m_pCamera->SetPlayer(this);
	m_pCamera->SetTimeLag(0.25f);
	m_pCamera->SetOffset(XMFLOAT3(0.0f, 40.0f, -40.0f));
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
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 10.0f;
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void CPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		CCamera* p3rdPersonCamera = m_pCamera;
		p3rdPersonCamera->SetLookAt(GetPosition());
	}
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift.z += 1;
		if (dwDirection & DIR_BACKWARD) xmf3Shift.z -= 1;		
		if (dwDirection & DIR_RIGHT) xmf3Shift.x += 1;
		if (dwDirection & DIR_LEFT) xmf3Shift.x -= 1;

		xmf3Shift = Vector3::Normalize(xmf3Shift);
		SetLookEnd(xmf3Shift);
		xmf3Shift = Vector3::Add(xmf3Shift, xmf3Shift, fDistance);
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Update(float fTimeElapsed)
{
	// now 는 현재, end는 목표 방향 벡터.
	// Look 벡터를 선형보간을 통해 부드럽게 회전.
	XMVECTOR now = XMLoadFloat3(&m_xmf3Look);
	XMVECTOR end = XMLoadFloat3(&m_xmf3LookEnd);
	float degree = XMConvertToDegrees(std::acos(Vector3::DotProduct(m_xmf3Look, m_xmf3LookEnd)));
	// 180도 회전할 떄 선형보간으로 하면 회전이 어색해짐. 아예 90도 이상 회전할 때는 선형보간 말고 직접 회전으로 처리
	if (degree == 180)
	{
		float angleInDegrees = fTimeElapsed * 50000;
		float angleInRadians = XMConvertToRadians(angleInDegrees);
		XMFLOAT4X4 rotationMatrix = Matrix4x4::Rotate(0, angleInRadians, 0);
		m_xmf3Look = Vector3::TransformCoord(m_xmf3Look, rotationMatrix);
	}
	else
	{
		m_xmf3Look = Vector3::Normalize(Vector3::XMVectorToFloat3(DirectX::XMVectorLerp(now, end, fTimeElapsed * 10)));
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	}

	// 중력, 이동 속력 계산
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

	// Terrain에서 Player와 Camera 계산
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	// 마찰력
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	// 애니메이션 블렌딩
	if (m_pSkinnedAnimationController)
	{
		float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
		if (m_bMoving)
		{
			m_pSkinnedAnimationController->SetTrackEnable(WALK, true);
			// m_pSkinnedAnimationController->SetTrackEnable(UPPER_GUN, true);
			walk = min(1.f, walk + 5.f * fTimeElapsed);
			idle = 1.f - walk;
		}
		else
		{
			idle = min(1.f, idle + 5.f * fTimeElapsed);
			walk = 1.f - idle;
		}
		if (idle == 1.f)
		{
			//m_pSkinnedAnimationController->SetTrackEnable(WALK, false);
			//m_pSkinnedAnimationController->SetTrackPosition(WALK, 0.0f);
		}
		//m_pSkinnedAnimationController->SetTrackWeight(IDLE, idle);
		//m_pSkinnedAnimationController->SetTrackWeight(WALK, walk);
	}
}