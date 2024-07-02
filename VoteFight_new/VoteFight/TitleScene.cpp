#include "pch.h"
#include "TitleScene.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "Texture.h"
#include "UI.h"
#include "Shader.h"
#include "State.h"
#include "StateMachine.h"
#include "Animator.h"
#include "Transform.h"
#include "Camera.h"
#include"Terrain.h"
#include "Scene.h"
#include "SceneManager.h"


CTitleScene* CTitleScene::m_CTitleScene;

CTitleScene::CTitleScene() :
	m_d3d12GameScene(),
	m_mappedGameScene()
{
	SetName("TitleScene");
}

CTitleScene::~CTitleScene()
{
	ReleaseShaderVariables();
}

void CTitleScene::CreateShaderVariables()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	UINT bytes = (sizeof(CB_TitleScene) + 255) & ~255;

	m_d3d12GameScene = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12GameScene->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedGameScene)));

	m_mappedGameScene->gcGlobalAmbientLight = XMFLOAT4(0, 1, 0, 1);
	m_mappedGameScene->m_fog.m_color = XMFLOAT4(0.025f, 0.025f, 0.05f, 1.0f);
	m_mappedGameScene->m_fog.m_density = 0.015f;
}

void CTitleScene::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::GAME_SCENE), m_d3d12GameScene->GetGPUVirtualAddress());
}

void CTitleScene::ReleaseShaderVariables()
{
	if (m_d3d12GameScene.Get() != nullptr)
	{
		m_d3d12GameScene->Unmap(0, nullptr);
	}
}

void CTitleScene::Enter()
{
	// ShowCursor(false);

	// ī�޶��� Ÿ�� ����
	const vector<CObject*>& objects = GetGroupObject(GROUP_TYPE::PLAYER);

	// CCameraManager::GetInstance()->GetMainCamera()->SetTarget(static_cast<CPlayer*>(objects[0]));
	// CSoundManager::GetInstance()->Play(SOUND_TYPE_INGAME_BGM_1, 0.3f, false);
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		for (const auto& object : objects)
		{
			if ((object->IsActive()) && (!object->IsDeleted()))
			{
				if (i == static_cast<int>(GROUP_TYPE::UI)) continue;
				if (m_terrain && (object->GetInstanceID() != (UINT)GROUP_TYPE::UI))object->InTerrainSpace(*this);

			}
		}
	}
}

void CTitleScene::Exit()
{
}

void CTitleScene::Init()
{
	// �� �ε�
	Load("GameScene.bin");
	LoadUI("TitleSceneUI.bin");

	CreateShaderVariables();

	// ����(Light) ����
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

}

// 2024 04 18�� �̽ÿ� ���� 
// Update�ҋ����� �÷��̾� Y �� �ͷ��� ���� ���� ��ȯ��Ŵ
void CTitleScene::Update()
{
	CScene::Update();

	if (KEY_TAP(KEY::SPACE))
	{
	 CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::SELECT);
	}
}

void CTitleScene::PreRender()
{
	UpdateShaderVariables();
}

void CTitleScene::Render()
{
	CScene::Render();
}

