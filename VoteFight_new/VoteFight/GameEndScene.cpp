#include "pch.h"
#include "GameEndScene.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "SoundManager.h"
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
#include "MainShader.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"

CGameEndScene* CGameEndScene::m_CGameEndScene;

CGameEndScene::CGameEndScene() :
	m_d3d12GameScene(),
	m_mappedGameScene(),
	m_selected_model(-1),
	m_button("Ready")
{
	SetName("SelectScene");
}

CGameEndScene::~CGameEndScene()
{
	ReleaseShaderVariables();
}

void CGameEndScene::CreateShaderVariables()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	UINT bytes = (sizeof(CB_GameEndScene) + 255) & ~255;

	m_d3d12GameScene = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12GameScene->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedGameScene)));

}

void CGameEndScene::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::GAME_SCENE), m_d3d12GameScene->GetGPUVirtualAddress());
}

void CGameEndScene::ReleaseShaderVariables()
{
	if (m_d3d12GameScene.Get() != nullptr)
	{
		m_d3d12GameScene->Unmap(0, nullptr);
	}
}

void CGameEndScene::Enter()
{
	InitLight();

	CCameraManager::GetInstance()->GetMainCamera()->SetTarget(nullptr);
	CCameraManager::GetInstance()->GetMainCamera()->SetPosition(XMFLOAT3(45,48,354));
	CCameraManager::GetInstance()->GetMainCamera()->SetRotate(XMFLOAT3(-1.5,-80,0));
	CCameraManager::GetInstance()->GetMainCamera()->SetOffset(XMFLOAT3(0, 0, 0));
	CSoundManager::GetInstance()->Play(SOUND_TYPE::SELECT_BGM, 0.2f, false);

}

void CGameEndScene::Exit()
{
	CSoundManager::GetInstance()->Stop(SOUND_TYPE::SELECT_BGM);
}

void CGameEndScene::Init()
{

	CObject* object = new CSkyBox(1000, 200);
	AddObject(GROUP_TYPE::SKYBOX, object, 0);

	// 씬 로드
	Load("GameEndObjectScene.bin");
	Load("GameEndCharters.bin");

	CreateShaderVariables();
}

void CGameEndScene::InitLight()
{
	// 조명(Light) 생성
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);		// 쉐도우 매핑만 해주는 조명. 실제로 조명을 물체에 적용시키진 않는다.
	m_mappedGameScene->m_lights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(0.0f, -1.f, 1.0f);	// Player 따라다님.
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 1.0f));
	m_mappedGameScene->m_lights[0].m_range = 500.f;
	cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);		// 물체에 조명을 적용시켜줌.
	m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_shadowMapping = false;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, -1.f, -1.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, -1.0f));
	m_mappedGameScene->m_lights[1].m_range = 500.f;
}


void CGameEndScene::Update()
{
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::UI); ++i)
	{
		for (auto& object : GetGroupObject((GROUP_TYPE)i))
		{
			if (object.second) object.second->Update();
		}
	}
}

void CGameEndScene::PreRender()
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
					const unordered_map<int, CObject*>& objects = GetGroupObject(static_cast<GROUP_TYPE>(i));

					for (const auto& object : objects)
					{
						if (object.second->GetName() != "SelectOcean" && (object.second->IsActive()) && (!object.second->IsDeleted()))
						{
							object.second->PreRender(camera);
						}
					}
				}

				DX::ResourceTransition(d3d12GraphicsCommandList, depthTexture->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
			}
		}
	}

	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();

	CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	CD3DX12_RECT rect = CD3DX12_RECT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &rect);

	CTexture* GBufferColor = CAssetManager::GetInstance()->GetTexture("GBufferColor");
	CTexture* GBufferNormal = CAssetManager::GetInstance()->GetTexture("GBufferNormal");
	CTexture* GBufferWorldPos = CAssetManager::GetInstance()->GetTexture("GBufferWorldPos");
	DX::ResourceTransition(commandList, GBufferColor->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DX::ResourceTransition(commandList, GBufferNormal->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DX::ResourceTransition(commandList, GBufferWorldPos->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
	{
		GBufferColor->m_RTVHandle.GetCPUHandle(),
		GBufferNormal->m_RTVHandle.GetCPUHandle(),
		GBufferWorldPos->m_RTVHandle.GetCPUHandle()
	};

	commandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, &framework->GetDepthStencilView());
	commandList->ClearRenderTargetView(rtvHandles[0], Colors::SkyBlue, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[1], Colors::SkyBlue, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[2], Colors::SkyBlue, 0, nullptr);

	GBufferColor->UpdateShaderVariable();
	GBufferNormal->UpdateShaderVariable();
	GBufferWorldPos->UpdateShaderVariable();

	UpdateShaderVariables();

	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();

	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (int i = 0; i < static_cast<int>(GROUP_TYPE::UI); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			object.second->Render(camera);
		}
	}

	DX::ResourceTransition(commandList, GBufferColor->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, GBufferNormal->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, GBufferWorldPos->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
}

void CGameEndScene::Render()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();

	CMainShader* shader = reinterpret_cast<CMainShader*>(CAssetManager::GetInstance()->GetShader("MainShader"));

	shader->SetPipelineState(0);

	CD3DX12_VIEWPORT vctResViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	commandList->RSSetViewports(1, &vctResViewport);
	commandList->RSSetScissorRects(1, &vctRect);

	commandList->IASetVertexBuffers(0, 1, &framework->mFullscreenQuadVertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	commandList->DrawInstanced(4, 1, 0, 0);

	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (const auto& object : m_objects[static_cast<int>(GROUP_TYPE::UI)])
	{
		if ((object.second->IsActive()) && (!object.second->IsDeleted()))
		{
			object.second->Render(camera);
		}
	}

	RenderImGui();
}

void CGameEndScene::RenderImGui()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	CGameFramework* framework = CGameFramework::GetInstance();
	framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

	ImVec2 windowSize(framework->GetResolution().x * 1 / 6, framework->GetResolution().y * 1 / 3);
	ImVec2 windowPos(framework->GetResolution().x * 1 / 6, framework->GetResolution().y * 1 / 3.3);

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // 투명 배경
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
	ImGui::GetFont()->Scale = 1.5;
	ImGui::PushFont(ImGui::GetFont());

	DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	ImGui::Begin("Selected", nullptr, window_flags);
	ImGui::Text("Selected");
	ImGui::SameLine();
	ImGui::End();

	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	static bool hovered[3] = { false };

	windowSize.x = windowSize.x / 1.3f;
	for (int i = 0; i < 3; ++i)
	{
		windowPos.x = framework->GetResolution().x * 1 / 2.3 + windowSize.x * i;
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

		ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 bgColor = ImVec4(0.f, 0.f, 0.f, 0.f);
		if (hovered[i]) borderColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		if (m_selected_id[i] != -1 || m_selected_model == i) {
			borderColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			if (m_selected_id[i] != -1) bgColor = ImVec4(1.0f, 0.0f, 0.0f, 0.3f);
		}

		ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor); // 투명 배경
		ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4.0f);
		ImGui::GetFont()->Scale = 1.5;
		ImGui::PushFont(ImGui::GetFont());

		ImGui::Begin(m_character_names[i].c_str(), nullptr, window_flags);

		ImGui::Text(m_character_names[i].c_str());
		ImGui::Text((": " + m_nicknames[i]).c_str());
		ImGui::SameLine();
		ImGui::End();

		ImGui::PopFont();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	windowPos.x = framework->GetResolution().x * 3 / 4;
	windowPos.y = framework->GetResolution().y * 3 / 4;
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); // 투명 배경
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::GetFont()->Scale = 2.0;
	ImGui::PushFont(ImGui::GetFont());

	ImGui::Begin("Button", nullptr, window_flags);
	{
		if (ImGui::Button(m_button.c_str(), ImVec2(framework->GetResolution().x / 5, framework->GetResolution().y / 7)) && !m_ready) {
				m_button = "Restart";
		}
		ImGui::End();
		ImGui::PopFont();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());

}

