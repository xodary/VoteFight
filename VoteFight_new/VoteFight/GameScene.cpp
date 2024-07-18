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
#include "State.h"
#include "StateMachine.h"
#include "Animator.h"
#include "Transform.h"
#include "Camera.h"
 #include"Terrain.h"
 #include"Bilboard.h"
#include "Monster.h"
#include "MainShader.h"
#include "./ImaysNet/ImaysNet.h"
#include "./ImaysNet/PacketQueue.h"
#include "TerrainObject.h"
#include "DropItem.h"
#include "NPC.h"
#include "Texture.h"

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

	CCameraManager::GetInstance()->SetGameSceneMainCamera();

	const unordered_map<int,CObject*>& objects = GetGroupObject(GROUP_TYPE::PLAYER);
	const unordered_map<int,CObject*>& Monsters = GetGroupObject(GROUP_TYPE::MONSTER);

	// CCameraManager::GetInstance()->GetMainCamera()->SetTarget(static_cast<CPlayer*>(objects[0]));
	// CSoundManager::GetInstance()->Play(SOUND_TYPE_INGAME_BGM_1, 0.3f, false);
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		for (const auto& object : objects)
		{
			if ((object.second->IsActive()) && (!object.second->IsDeleted()))
			{
				if (i == static_cast<int>(GROUP_TYPE::UI)) continue;
				if (m_terrain && (object.second->GetInstanceID() != (UINT)GROUP_TYPE::UI))
					object.second->InTerrainSpace(*this);
			}
		}
	}

	InitLight();
}

void CGameScene::Exit()
{
}

void CGameScene::Init()
{
	m_terrain = CTerrainObject::Load("HeightMap");

	//SceneLoad();
	Load("GameScene.bin");
	Load("GameScene2.bin");


	CObject* object = new CSkyBox(1000, 200);
	AddObject(GROUP_TYPE::SKYBOX, object, 0);

	//CObject* playerObject = CObject::Load("hugo_idle");

	//{
	//	CObject* gun = CObject::Load("Gun");
	//	CTransform* transform = reinterpret_cast<CTransform*>(gun->GetComponent(COMPONENT_TYPE::TRANSFORM));
	//	transform->SetPosition(XMFLOAT3(10, 3.0f, 10));
	//	AddObject(GROUP_TYPE::GROUND_ITEM, gun);
	//}

	//CTransform* transform = reinterpret_cast<CTransform*>(playerObject->GetComponent(COMPONENT_TYPE::TRANSFORM));
	//transform->SetPosition(XMFLOAT3(20, 2.37f, 20));
	//AddObject(GROUP_TYPE::PLAYER, playerObject);
	//CPlayer* player = reinterpret_cast<CPlayer*>(playerObject);
	//player->Init();

	//object = CObject::Load("Marge_Police");
	//transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	//transform->SetPosition(XMFLOAT3(30, 2.37f, 30));
	//AddObject(GROUP_TYPE::NPC, object);
	//CNPC* npc = reinterpret_cast<CNPC*>(object);
	//npc->Init();

	// Collision Check
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::STRUCTURE);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::NPC);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::MONSTER);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::BOX);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::ONCE_ITEM);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::GROUND_ITEM);

	CreateShaderVariables();
}

void CGameScene::InitLight()
{
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);		// ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ ÔøΩÔøΩÔøΩŒ∏ÔøΩ ÔøΩÔøΩÔøΩ÷¥ÔøΩ ÔøΩÔøΩÔøΩÔøΩ. ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ ÔøΩÔøΩ√ºÔøΩÔøΩ ÔøΩÔøΩÔøΩÔøΩÔøΩ≈∞ÔøΩÔø?ÔøΩ ¥¬¥ÔøΩ.
	m_mappedGameScene->m_lights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(0.0f, 1.0f, 1.0f);	// Player ÔøΩÔøΩÔøΩÔøΩŸ¥Ôø?
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 1.0f));
	m_mappedGameScene->m_lights[0].m_range = 100.f;
	cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);		// ÔøΩÔøΩ√ºÔøΩÔøΩ ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔø?
	m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_shadowMapping = false;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 1.0f, -1.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, -1.0f));
	m_mappedGameScene->m_lights[1].m_range = 100.f;

	//CCameraManager::GetInstance()->GetMainCamera()->SetTarget(player);

}

float Lerp(float A, float B, float Alpha)
{
	A = (int)A % 360;
	B = (int)B % 360;
	if ((A - B) > 180) B += 360;
	else if ((B - A) > 180) A += 360;
	float result = A * (1 - Alpha) + B * Alpha;
	return result;
}

void CGameScene::Update()
{
	unordered_map<int, CObject*> objects = GetGroupObject(GROUP_TYPE::PLAYER);

	for (auto& object : objects) {

		if (KEY_TAP(KEY::NUM1))
			reinterpret_cast<CPlayer*>(object.second)->SwapWeapon(WEAPON_TYPE::PISTOL);
		if (KEY_TAP(KEY::NUM2))
			reinterpret_cast<CPlayer*>(object.second)->SwapWeapon(WEAPON_TYPE::AXE);
		if (KEY_TAP(KEY::NUM3))
			reinterpret_cast<CPlayer*>(object.second)->SwapWeapon(WEAPON_TYPE::PUNCH);

		CTransform* p_tf = static_cast<CTransform*>(object.second->GetComponent(COMPONENT_TYPE::TRANSFORM));

		float rotation = Lerp(p_tf->GetRotation().y, reinterpret_cast<CPlayer*>(object.second)->goal_rota, DT * 8);
		p_tf->SetRotation(XMFLOAT3(0, rotation, 0));

		XMFLOAT3 pos = p_tf->GetPosition();
		if (0 <= (int)pos.x && (int)pos.x < 400 && 0 <= (int)pos.z && (int)pos.z < 400)
			p_tf->SetPosition(XMFLOAT3(pos.x, GetTerrainHeight(pos.x, pos.z), pos.z));
	}

	unordered_map<int, CObject*> items = GetGroupObject(GROUP_TYPE::GROUND_ITEM);
	for (auto& item : items) {
		CTransform* i_tf = static_cast<CTransform*>(item.second->GetComponent(COMPONENT_TYPE::TRANSFORM));
		i_tf->Rotate(XMFLOAT3(0, DT * 100.f, 0));
		XMFLOAT3 p = i_tf->GetPosition();
		if (0 <= (int)p.x && (int)p.x < 400 && 0 <= (int)p.z && (int)p.z < 400) {
			float h = cos(CGameFramework::GetInstance()->m_mappedGameFramework->m_totalTime) * 0.3f;
			i_tf->SetPosition(XMFLOAT3(p.x, GetTerrainHeight(p.x, p.z) + 1.2f + h, p.z));
		}
	}

	CObject* object = GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id);
	m_mappedGameScene->m_lights[0].m_position = Vector3::Add(object->GetPosition(), XMFLOAT3(-25.f, 0, -25.f));
	m_mappedGameScene->m_lights[1].m_position = object->GetPosition();
	CScene::Update();
}

void CGameScene::PreRender()
{
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

				d3d12GraphicsCommandList->ClearRenderTargetView(depthTexture->m_RTVHandle.GetCPUHandle(), Colors::White, 0, nullptr);
				d3d12GraphicsCommandList->ClearDepthStencilView(depthTexture->m_DSVHandle.GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
				d3d12GraphicsCommandList->OMSetRenderTargets(1, &depthTexture->m_RTVHandle.GetCPUHandle(), TRUE, &depthTexture->m_DSVHandle.GetCPUHandle());

				camera->RSSetViewportsAndScissorRects();
				camera->UpdateShaderVariables();
				depthTexture->UpdateShaderVariable();

				for (auto& object : GetViewList(1))
				{
					if ((object->IsActive()) && (!object->IsDeleted()))
					{
						object->PreRender(camera);
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
	commandList->ClearRenderTargetView(rtvHandles[0], Colors::White, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[1], Colors::White, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[2], Colors::White, 0, nullptr);

	GBufferColor->UpdateShaderVariable();
	GBufferNormal->UpdateShaderVariable();
	GBufferWorldPos->UpdateShaderVariable();

	UpdateShaderVariables();
	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();

	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (auto& object : GetViewList(0))
	{
		if ((object->IsActive()) && (!object->IsDeleted()))
		{
			object->Render(camera);
		}
	}

	DX::ResourceTransition(commandList, GBufferColor->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, GBufferNormal->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, GBufferWorldPos->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
}

void CGameScene::Render()
{	
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();

	CMainShader* shader = reinterpret_cast<CMainShader*>(CAssetManager::GetInstance()->GetShader("MainShader"));

	shader->SetPipelineState(0);

	UpdateShaderVariables();

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
	
	for (auto& object : GetViewList(0))
	{
		object->RenderBilboard(camera);
	}

	if (KEY_HOLD(KEY::SPACE))
	{
		// [Debug] Render DepthTexture
		const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();
		D3D12_VIEWPORT d3d12Viewport = { 0.0f, 0.0f, resolution.x * 0.4f, resolution.y * 0.4f, 0.0f, 1.0f };
		D3D12_RECT d3d12ScissorRect = { 0, 0,(LONG)(resolution.x * 0.4f), (LONG)(resolution.y * 0.4f) };
		CTexture* texture = CAssetManager::GetInstance()->GetTexture("DepthWrite");
		CShader* shader = CAssetManager::GetInstance()->GetShader("DepthWrite");

		texture->UpdateShaderVariable();
		shader->SetPipelineState(2);
		commandList->RSSetViewports(1, &d3d12Viewport);
		commandList->RSSetScissorRects(1, &d3d12ScissorRect);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->DrawInstanced(6, 1, 0, 0);
	}

	RenderImGui();

	// for (auto object : m_objects_id) {
	// 	if (object.second != nullptr)
	// 		object.second->RenderUI(camera);
	// }
	// RenderChatUI();
}

void CGameScene::RenderImGui()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	CPlayer* player = reinterpret_cast<CPlayer*>(GetIDObject(GROUP_TYPE::PLAYER, framework->my_id));
	framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (KEY_HOLD(KEY::E)) {

		int rows = 3;
		int cols = 6;
		static bool selected[3][6] = { false };
		static bool hovered[3][6] = { false };
		const ImVec4 hoverColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // ø‹∞˚º± ªˆªÛ (ª°∞≠)

		ImVec2 windowSize(framework->GetResolution().x * 3 / 4, framework->GetResolution().y * 3 / 4);
		ImVec2 windowPos((framework->GetResolution().x - windowSize.x) / 2, (framework->GetResolution().y - windowSize.y) / 2);

		DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::Begin("Full Screen Window", nullptr, window_flags);

		// æ∆¿Ã≈€ ≈©±‚ ∞ËªÍ
		const float itemSize = windowSize.x / 8;
		const float spacing = itemSize / 6;
		const float totalSpacingWidth = (cols - 1) * spacing;
		const float totalSpacingHeight = (rows - 1) * spacing;
		const float inventoryWidth = cols * itemSize + totalSpacingWidth;
		const float inventoryHeight = rows * itemSize + totalSpacingHeight;

		float startX = (windowSize.x - inventoryWidth) / 2.0f;
		float startY = (windowSize.y - inventoryHeight) / 2.0f;


		ImGui::SetCursorPosY(startY);

		for (int i = 0; i < rows; ++i)
		{
			ImGui::SetCursorPosX(startX);

			for (int j = 0; j < cols; ++j)
			{
				ImGui::PushID(i * cols + j);

				if (j > 0) // √π π¯¬∞ æ∆¿Ã≈€¿Ã æ∆¥— ∞ÊøÏø°∏∏ ∞£∞› √ﬂ∞°
					ImGui::SameLine(0.0f, spacing);

				// ø‹∞˚º± ªˆªÛ º≥¡§
				ImVec4 borderColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
				if (hovered[i][j]) borderColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				if (selected[i][j]) borderColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

				ImGui::GetFont()->Scale = 1.0f;
				ImGui::PushFont(ImGui::GetFont());
				ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

				ImGui::BeginChildFrame(ImGui::GetID((void*)(intptr_t)(i * cols + j)), ImVec2(itemSize, itemSize));
				{
					if (ImGui::IsWindowHovered()) {
						if (ImGui::GetIO().MouseClicked[0])
							selected[i][j] = !selected[i][j];
						else
							hovered[i][j] = true;
					}
					else
						hovered[i][j] = false;

					if (!player->myItems[i * cols + j].empty()) {
						int item = CAssetManager::GetInstance()->m_IconTextures.count(player->myItems[i * cols + j]);
						if (item != 0) {
							auto& handle = CAssetManager::GetInstance()->m_IconTextures[player->myItems[i * cols + j]]->m_IconGPUHandle;
							ImGui::Image((void*)handle.ptr, ImVec2(itemSize * 2 / 3, itemSize * 2 / 3));
						}
						ImGui::Text("%s", player->myItems[i * cols + j].c_str());
					}
					ImGui::EndChildFrame();
				}

				ImGui::PopStyleVar();
				ImGui::PopStyleColor();
				ImGui::PopFont();
				ImGui::PopID();

				//if (j < cols - 1) ImGui::SameLine();
				if (i < cols - 1)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
			}
		}

		ImGui::End();
	}

	int rows = 3;
	static bool selected[3] = { false };
	const ImVec4 hoverColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // ø‹∞˚º± ªˆªÛ (ª°∞≠)

	ImVec2 windowSize(framework->GetResolution().x * 1 / 4, framework->GetResolution().x * 1 / 4 / 3);
	ImVec2 windowPos((framework->GetResolution().x - windowSize.x) / 2, (framework->GetResolution().y - windowSize.y));

	DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::Begin("Full Screen Window", nullptr, window_flags);


	// æ∆¿Ã≈€ ≈©±‚ ∞ËªÍ
	const float itemSize = windowSize.y * 5 / 6;

	ImGui::SetCursorPosX(itemSize / 5);
	for (int i = 0; i < rows; ++i)
	{
		ImGui::PushID(i+ 1000);

		// ø‹∞˚º± ªˆªÛ º≥¡§
		ImVec4 borderColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		if (selected[i]) borderColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		ImGui::GetFont()->Scale = 1.0f;
		ImGui::PushFont(ImGui::GetFont());
		ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);


		ImGui::BeginChildFrame(ImGui::GetID((void*)(intptr_t)(i+1000)), ImVec2(itemSize, itemSize));
		{
			ImGui::Text("%d", i + 1);
			if (!player->myItems[i].empty()) {
				int item = CAssetManager::GetInstance()->m_IconTextures.count(player->myItems[i]);
				if (item != 0) {
					ImGui::SameLine();
					auto& handle = CAssetManager::GetInstance()->m_IconTextures[player->myItems[i]]->m_IconGPUHandle;
					ImGui::Image((void*)handle.ptr, ImVec2(itemSize * 2 / 3, itemSize * 2 / 3));
				}
			}
			ImGui::Text(player->myItems[i].c_str());
			ImGui::EndChildFrame();
		}
		ImGui::SameLine();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::PopID();
	}
	ImGui::End();

	windowSize.x = framework->GetResolution().x / 5;
	windowSize.y = framework->GetResolution().y / 15;
	windowPos.x = framework->GetResolution().x - windowSize.x - 10;
	windowPos.y = 10;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 1.f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

	ImGui::Begin("White Background Window", nullptr, window_flags);
	{
		ImVec2 center = ImVec2(windowPos.x + windowSize.x / 2, windowPos.y + windowSize.y / 2);
		
		float rate = CTimeManager::GetInstance()->m_lastTime / CTimeManager::GetInstance()->m_phaseTime;
		ImVec2 top_left = ImVec2(center.x - windowSize.x / 2, center.y - windowSize.y / 2);
		ImVec2 bottom_right = ImVec2((center.x - windowSize.x / 2) + windowSize.x * rate, center.y + windowSize.y / 2);

		// ªÁ∞¢«¸ ±◊∏Æ±‚
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(top_left, bottom_right, IM_COL32(0, 0, 0, 255));

	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());
}
