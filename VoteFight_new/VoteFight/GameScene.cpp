#include "pch.h"
#include "Skybox.h"
#include "GameScene.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "Texture.h"
#include "UI.h"
#include "Shader.h"
#include "StateMachine.h"
#include "Animator.h"
#include "Transform.h"
#include "Camera.h"
 #include"Terrain.h"
 #include"Bilboard.h"

#include "./ImaysNet/ImaysNet.h"
#include "./ImaysNet/PacketQueue.h"

CGameScene* CGameScene::m_CGameScene;

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

	m_mappedGameScene->gcGlobalAmbientLight = XMFLOAT4(0,1,0,1);
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
	CreateTerrain();

	// 씬 로드
	Load("GameScene.bin");
	Load("BinaryScene.bin");
	Load("Woods.bin");
	Load("FenceScene.bin");
	Load("Homer_link_Scene.bin");
	LoadUI("GameSceneUI.bin");

	// 스카이박스 추가
	CObject* object = new CSkyBox();
	AddObject(GROUP_TYPE::SKYBOX, object);

	// 2024 - 04 - 10
	// 빌보드 추가 - 아직 미완성
	//object = new CBilboard();
	//AddObject(GROUP_TYPE::BILBOARD, object);

	//// 충돌 그룹 설정
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::STRUCTURE);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::NPC);

	CreateShaderVariables();

	// 조명(Light) 생성
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);		// 쉐도우 매핑만 해주는 조명. 실제로 조명을 물체에 적용시키진 않는다.
	m_mappedGameScene->m_lights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(1.0f, 1.0f, 1.0f);	// Player 따라다님.
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 1.0f));
	m_mappedGameScene->m_lights[0].m_range = 500.f;
	cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);		// 물체에 조명을 적용시켜줌.
	m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_shadowMapping = false;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, -1.0f));
	m_mappedGameScene->m_lights[1].m_range = 500.f;

	/*
	m_towerLightAngle = XMConvertToRadians(90.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::SPOT);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 50.0f, 0.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(cosf(m_towerLightAngle), -1.0f, sinf(m_towerLightAngle)));
	m_mappedGameScene->m_lights[1].m_color = XMFLOAT4(0.7f, 0.7f, 0.3f, 0.0f);
	m_mappedGameScene->m_lights[1].m_attenuation = XMFLOAT3(0.5f, 0.01f, 0.0f);
	m_mappedGameScene->m_lights[1].m_fallOff = 1.0f;
	m_mappedGameScene->m_lights[1].m_range = 500.0f;
	m_mappedGameScene->m_lights[1].m_theta = cosf(XMConvertToRadians(5.0f));
	m_mappedGameScene->m_lights[1].m_phi = cosf(XMConvertToRadians(10.0f));

	m_mappedGameScene->m_lights[2].m_isActive = true;
	m_mappedGameScene->m_lights[2].m_type = static_cast<int>(LIGHT_TYPE::POINT);
	m_mappedGameScene->m_lights[2].m_position = XMFLOAT3(7.5f * cosf(m_towerLightAngle), 37.0f, 7.5f * sinf(m_towerLightAngle));
	m_mappedGameScene->m_lights[2].m_color = XMFLOAT4(1.0f, 1.0f, 0.8f, 0.0f);
	m_mappedGameScene->m_lights[2].m_attenuation = XMFLOAT3(0.5f, 0.01f, 0.0f);
	m_mappedGameScene->m_lights[2].m_range = 7.0f;
	*/
	vector<CObject*> objects = GetGroupObject(GROUP_TYPE::STRUCTURE);
	CCameraManager::GetInstance()->GetMainCamera()->SetTarget(objects[0]);
}

// 2024 04 18일 이시영 수정 
// Update할떄마다 플레이어 Y 를 터레인 높이 값에 변환시킴
void CGameScene::Update()
{
	CObject* player = GetGroupObject(GROUP_TYPE::STRUCTURE)[0];
	CPlayer* my_player = reinterpret_cast<CPlayer*>(player);
	CTransform*	net_transform = static_cast<CTransform*>(my_player->GetComponent(COMPONENT_TYPE::TRANSFORM));
	CStateMachine* net_stateMachine = static_cast<CStateMachine*>(my_player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	net_stateMachine->GetCurrentState();

	CS_MOVE_V_PACKET send_packet;
	send_packet.m_size = sizeof(CS_MOVE_V_PACKET);
	send_packet.m_type = PACKET_TYPE::P_CS_MOVE_V_PACKET;
	send_packet.m_id = my_player->m_id;
	send_packet.m_vec = net_transform->GetPosition();
	send_packet.m_rota = net_transform->GetRotation();
	// send_packet.m_state = net_stateMachine->;
	PacketQueue::AddSendPacket(&send_packet);
	// cout << " >> send ) CS_MOVE_V_PACKET" << endl;
	
	vector<CObject*> objects = GetGroupObject(GROUP_TYPE::PLAYER);
	for (CObject* o : objects) {
		CPlayer* player = reinterpret_cast<CPlayer*>(o);
		if (player->m_id == CGameFramework::GetInstance()->my_id) {
			player->SetTerrainY(this);
			CTransform* playerPosition = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
			m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(playerPosition->GetPosition().x, playerPosition->GetPosition().y + 10, playerPosition->GetPosition().z);
		}
	}
	CScene::Update();
}

void CGameScene::PreRender()
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
					//camera->GenerateOrthographicsProjectionMatrix(static_cast<float>(TERRAIN_WIDTH), static_cast<float>(TERRAIN_HEIGHT), nearPlaneDist, farPlaneDist);
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

				CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12RtvCPUDescriptorHandle(CGameFramework::GetInstance()->GetRtvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());
				CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DsvCPUDescriptorHandle(CGameFramework::GetInstance()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());

				d3d12RtvCPUDescriptorHandle.ptr += 2 * CGameFramework::GetInstance()->GetRtvDescriptorIncrementSize();
				d3d12DsvCPUDescriptorHandle.ptr += CGameFramework::GetInstance()->GetDsvDescriptorIncrementSize();

				d3d12GraphicsCommandList->ClearRenderTargetView(d3d12RtvCPUDescriptorHandle, Colors::White, 0, nullptr);
				d3d12GraphicsCommandList->ClearDepthStencilView(d3d12DsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
				d3d12GraphicsCommandList->OMSetRenderTargets(1, &d3d12RtvCPUDescriptorHandle, TRUE, &d3d12DsvCPUDescriptorHandle);

				camera->RSSetViewportsAndScissorRects();
				camera->UpdateShaderVariables();
				depthTexture->UpdateShaderVariable();

				for (int i = static_cast<int>(GROUP_TYPE::STRUCTURE); i <= static_cast<int>(GROUP_TYPE::PLAYER); ++i)
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

void CGameScene::Render()
{
	CScene::Render();

	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	// [Debug] Render DepthTexture
	const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();
	D3D12_VIEWPORT d3d12Viewport = { 0.0f, 0.0f, resolution.x * 0.4f, resolution.y * 0.4f, 0.0f, 1.0f };
	D3D12_RECT d3d12ScissorRect = { 0, 0,(LONG)(resolution.x * 0.4f), (LONG)(resolution.y * 0.4f) };
	CTexture* texture = CAssetManager::GetInstance()->GetTexture("DepthWrite");
	CShader* shader = CAssetManager::GetInstance()->GetShader("DepthWrite");

	texture->UpdateShaderVariable();
	shader->SetPipelineState(2);
	d3d12GraphicsCommandList->RSSetViewports(1, &d3d12Viewport);
	d3d12GraphicsCommandList->RSSetScissorRects(1, &d3d12ScissorRect);
	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d12GraphicsCommandList->DrawInstanced(6, 1, 0, 0);

}

