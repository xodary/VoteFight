#include "pch.h"
#include "SelectScene.h"
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
#include "Skybox.h"

CSelectScene* CSelectScene::m_CSelectScene;

CSelectScene::CSelectScene() :
	m_d3d12GameScene(),
	m_mappedGameScene()
{
	SetName("SelectScene");
}

CSelectScene::~CSelectScene()
{
	ReleaseShaderVariables();
}

void CSelectScene::CreateShaderVariables()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	UINT bytes = (sizeof(CB_SelectScene) + 255) & ~255;

	m_d3d12GameScene = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12GameScene->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedGameScene)));

}

void CSelectScene::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::GAME_SCENE), m_d3d12GameScene->GetGPUVirtualAddress());
}

void CSelectScene::ReleaseShaderVariables()
{
	if (m_d3d12GameScene.Get() != nullptr)
	{
		m_d3d12GameScene->Unmap(0, nullptr);
	}
}

void CSelectScene::Enter()
{
	// 카메라의 타겟 설정
	const vector<CObject*>& objects = GetGroupObject(GROUP_TYPE::PLAYER);

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

void CSelectScene::Exit()
{
}

void CSelectScene::Init()
{
	// 씬 로드
	Load("GameScene.bin");
	Load("Character_Scene.bin");

	CreateShaderVariables();

	InitLight();

	CCameraManager::GetInstance()->SetSelectSceneMainCamera();

	vector<CObject*> objects = GetGroupObject(GROUP_TYPE::STRUCTURE);
	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->SetDeleted(true);
	}

	 objects = GetGroupObject(GROUP_TYPE::PLAYER);
	m_SelectCharacter = objects[0];
	m_SelectCharacter->SetRotate(XMFLOAT3(0, 180, 0));

	for (size_t i = 0; i < objects.size(); i++)
	{
		m_WaitCharacters[i] = objects[i];
		m_WaitCharacters[i]->SetPostion(XMFLOAT3(4 + i * 4, 1, 0.3));
		m_WaitCharacters[i]->SetRotate(XMFLOAT3(0, 180, 0));
		cout << m_WaitCharacters[i]->GetName() << endl;
	}

}

void CSelectScene::InitLight()
{
	// 조명(Light) 생성
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);		// 쉐도우 매핑만 해주는 조명. 실제로 조명을 물체에 적용시키진 않는다.
	m_mappedGameScene->m_lights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(0.0f, 1.0f, 1.0f);	// Player 따라다님.
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 1.0f));
	m_mappedGameScene->m_lights[0].m_range = 500.f;
	cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);		// 물체에 조명을 적용시켜줌.
	m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_shadowMapping = false;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 1.0f, -1.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, -1.0f));
	m_mappedGameScene->m_lights[1].m_range = 500.f;
}

void CSelectScene::SelectCharacter(UINT number)
{
	XMFLOAT3 tempPostion = m_SelectCharacter->GetPostion();
	
	for (size_t i = 0; i < 3; i++)
	{
		if (number != i)
		{
			m_WaitCharacters[i]->SetPostion(XMFLOAT3(4 + i * 2, 1, 0.3));
		}
		else
		{
			m_WaitCharacters[i]->SetPostion(XMFLOAT3(0, 1, 0.3));
		}
	}
}

// 2024 04 18일 이시영 수정 
// Update할떄마다 플레이어 Y 를 터레인 높이 값에 변환시킴
void CSelectScene::Update()
{
	CScene::Update();

	if (KEY_TAP(KEY::NUM1))
	{
		SelectCharacter(0);
	}
	if (KEY_TAP(KEY::NUM2))
	{
		SelectCharacter(1);
	}

	if (KEY_TAP(KEY::NUM3))
	{
		SelectCharacter(2);
	}

	if (KEY_TAP(KEY::SPACE))
	{
		// 서버에 send (m_SelectCharacter)의 이름이나 번호를 전달해서 GameScene의 플레이어 설정
		CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::GAME);
	}
}

void CSelectScene::PreRender()
{
	UpdateShaderVariables();

	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	for (const auto& camera : cameras)
	{
		if (camera->GetType() == CAMERA_TYPE::LIGHT)
		{
			LIGHT* light = camera->GetLight();

			if ((light != nullptr) && (light->m_isActive) && (light->m_shadowMapping))
			{
				float nearPlaneDist = 0.0;
				float farPlaneDist = light->m_range;

				switch ((LIGHT_TYPE)light->m_type)
				{
				case LIGHT_TYPE::POINT:
					break;
				case LIGHT_TYPE::SPOT:
					camera->GeneratePerspectiveProjectionMatrix(90.0f, static_cast<float>(DEPTH_BUFFER_WIDTH) / static_cast<float>(DEPTH_BUFFER_HEIGHT), nearPlaneDist, farPlaneDist);
					break;
				case LIGHT_TYPE::DIRECTIONAL:
					camera->GenerateOrthographicsProjectionMatrix(static_cast<float>(100), static_cast<float>(100), nearPlaneDist, farPlaneDist);
					break;
				}

				camera->GenerateViewMatrix(light->m_position, light->m_direction);

				XMFLOAT4X4 projectionToTexture =
				{
					0.5f,  0.0f, 0.0f, 0.0f,
					0.0f, -0.5f, 0.0f, 0.0f,
					0.0f,  0.0f, 1.0f, 0.0f,
					0.5f,  0.5f, 0.0f, 1.0f
				};

				XMStoreFloat4x4(&light->m_toTexCoord, XMMatrixTranspose(XMLoadFloat4x4(&camera->GetViewMatrix()) * XMLoadFloat4x4(&camera->GetProjectionMatrix()) * XMLoadFloat4x4(&projectionToTexture)));

				CTexture* depthTexture = CAssetManager::GetInstance()->GetTexture("DepthWrite");

				DX::ResourceTransition(d3d12GraphicsCommandList, depthTexture->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

				d3d12GraphicsCommandList->ClearRenderTargetView(depthTexture->m_RTVHandle.GetCPUHandle(), Colors::White, 0, nullptr);
				d3d12GraphicsCommandList->ClearDepthStencilView(depthTexture->m_DSVHandle.GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
				d3d12GraphicsCommandList->OMSetRenderTargets(1, &depthTexture->m_RTVHandle.GetCPUHandle(), TRUE, &depthTexture->m_DSVHandle.GetCPUHandle());

				camera->RSSetViewportsAndScissorRects();
				camera->UpdateShaderVariables();
				depthTexture->UpdateShaderVariable();

				for (int i = static_cast<int>(GROUP_TYPE::STRUCTURE); i <= static_cast<int>(GROUP_TYPE::BULLET); ++i)
				{
					const vector<CObject*>& objects = GetGroupObject(static_cast<GROUP_TYPE>(i));

					for (const auto& object : objects)
					{
						if ((object->IsActive()) && (!object->IsDeleted()))
						{
							object->PreRender(camera);
						}
					}
				}

				m_terrain->PreRender(camera);

				DX::ResourceTransition(d3d12GraphicsCommandList, depthTexture->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
			}
		}
	}
}

void CSelectScene::Render()
{
	CScene::Render();

}

