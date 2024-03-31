#include "pch.h"
#include "Skybox.h"
#include "GameScene.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "Player.h"
#include "Texture.h"
#include "UI.h"
#include "Shader.h"
#include "StateMachine.h"
#include "Animator.h"
#include "Transform.h"
#include "Camera.h"
 #include"Terrain.h"

CGameScene::CGameScene() :
	m_d3d12GameScene(),
	m_mappedGameScene(),
	m_towerLight(),
	m_towerLightAngle()
{
	SetName("GameScene");
}

CGameScene::~CGameScene()
{
	ReleaseShaderVariables();
}

void CGameScene::CreateShaderVariables()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	UINT bytes = (sizeof(CB_GameScene) + 255) & ~255;

	m_d3d12GameScene = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12GameScene->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedGameScene)));

	m_mappedGameScene->m_fog.m_color = XMFLOAT4(0.025f, 0.025f, 0.05f, 1.0f);
	m_mappedGameScene->m_fog.m_density = 0.015f;
}

void CGameScene::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::GAME_SCENE), m_d3d12GameScene->GetGPUVirtualAddress());
}

void CGameScene::ReleaseShaderVariables()
{
	if (m_d3d12GameScene.Get() != nullptr)
	{
		m_d3d12GameScene->Unmap(0, nullptr);
	}
}

void CGameScene::Enter()
{
	// ShowCursor(false);

	// 카메라의 타겟 설정
	const vector<CObject*>& objects = GetGroupObject(GROUP_TYPE::PLAYER);

	// CCameraManager::GetInstance()->GetMainCamera()->SetTarget(static_cast<CPlayer*>(objects[0]));
	// CSoundManager::GetInstance()->Play(SOUND_TYPE_INGAME_BGM_1, 0.3f, false);
}

void CGameScene::Exit()
{
}

void CGameScene::Init()
{
	// 씬 로드
	Load("GameScene.bin");
	LoadUI("GameSceneUI.bin");

	// 스카이박스 추가
	CObject* object = new CSkyBox();
	AddObject(GROUP_TYPE::SKYBOX, object);

	// 터레인 추가
	object = new CTerrain(257.f, 257.f);
	AddObject(GROUP_TYPE::TERRAIN, object);

	//// 충돌 그룹 설정
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::STRUCTURE);

	CreateShaderVariables();

	// 조명(Light) 생성
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(0.0f, 100.0f, 130.0f);
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, -1.0f));
	m_mappedGameScene->m_lights[0].m_color = XMFLOAT4(0.5f, 0.5f, 0.6f, 0.0f);
	m_mappedGameScene->m_lights[0].m_range = 2000.0f;
	// cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	vector<CObject*> objects = GetGroupObject(GROUP_TYPE::PLAYER);
	CCameraManager::GetInstance()->GetMainCamera()->SetTarget(objects[0]);
}

void CGameScene::Update()
{
	CScene::Update();
}

void CGameScene::PreRender()
{
	UpdateShaderVariables();

	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();
}

void CGameScene::Render()
{
	CScene::Render();
}

