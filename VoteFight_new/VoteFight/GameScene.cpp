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
#include "ImGUI/implot.h"
#include "ImGUI/implot_internal.h"
#include "SceneManager.h"
#include "LoginScene.h"
#include "SelectScene.h"

CGameScene* CGameScene::m_CGameScene;

CGameScene::CGameScene() :
	m_d3d12GameScene(),
	m_mappedGameScene()
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
	CCameraManager::GetInstance()->SetGameSceneMainCamera();
	InitLight();
}

void CGameScene::Exit()
{
}

void CGameScene::Init()
{
	m_terrain = CTerrainObject::Load("HeightMap");

	Load("GameScene.bin");

	CObject* object = new CSkyBox(1000, 200);
	AddObject(GROUP_TYPE::SKYBOX, object, 0);

	// Collision Check
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::STRUCTURE);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::NPC);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::MONSTER);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::BOX);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::ONCE_ITEM);
	CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::GROUND_ITEM);

	CreateShaderVariables();

	InitLight();
}

void CGameScene::InitLight()
{
	const vector<CCamera*>& cameras = CCameraManager::GetInstance()->GetCameras();

	m_mappedGameScene->m_lights[0].m_xmf4Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);	
	m_mappedGameScene->m_lights[0].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_mappedGameScene->m_lights[0].m_xmf4Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_mappedGameScene->m_lights[0].m_isActive = true;
	m_mappedGameScene->m_lights[0].m_shadowMapping = true;
	m_mappedGameScene->m_lights[0].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[0].m_position = XMFLOAT3(0.0f, 1.0f, 1.0f);	
	m_mappedGameScene->m_lights[0].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 1.0f));
	m_mappedGameScene->m_lights[0].m_range = 100.f;
	cameras[2]->SetLight(&m_mappedGameScene->m_lights[0]);

	m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);	
	m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f);
	m_mappedGameScene->m_lights[1].m_isActive = true;
	m_mappedGameScene->m_lights[1].m_shadowMapping = false;
	m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
	m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 1.0f, -1.0f);
	m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, -1.0f));
	m_mappedGameScene->m_lights[1].m_range = 100.f;
}

void CGameScene::Update()
{
	if (CTimeManager::GetInstance()->m_phase % 2 == 1) {
		m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else {
		m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
		m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
		m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.13f, 0.13f, 0.13f, 1.0f);
	}
	for (auto& object : GetGroupObject(GROUP_TYPE::PLAYER)) {
		CTransform* p_tf = static_cast<CTransform*>(object.second->GetComponent(COMPONENT_TYPE::TRANSFORM));

		float rotation = Lerp(p_tf->GetRotation().y, reinterpret_cast<CPlayer*>(object.second)->goal_rota, DT * 8);
		p_tf->SetRotation(XMFLOAT3(0, rotation, 0));

		XMFLOAT3 pos = p_tf->GetPosition();
		if (0 <= (int)pos.x && (int)pos.x < 400 && 0 <= (int)pos.z && (int)pos.z < 400)
			p_tf->SetPosition(XMFLOAT3(pos.x, GetTerrainHeight(pos.x, pos.z), pos.z));
	}

	for (auto& object : GetGroupObject(GROUP_TYPE::MONSTER)) {
		CTransform* p_tf = static_cast<CTransform*>(object.second->GetComponent(COMPONENT_TYPE::TRANSFORM));

		float rotation = Lerp(p_tf->GetRotation().y, reinterpret_cast<CMonster*>(object.second)->goal_rota, DT * 8);
		p_tf->SetRotation(XMFLOAT3(0, rotation, 0));

		XMFLOAT3 pos = p_tf->GetPosition();
		if (0 <= (int)pos.x && (int)pos.x < 400 && 0 <= (int)pos.z && (int)pos.z < 400)
			p_tf->SetPosition(XMFLOAT3(pos.x, GetTerrainHeight(pos.x, pos.z), pos.z));
	}
	CObject* object;
	if(CSceneManager::GetInstance()->GetCurrentScene()->m_name == "GameScene")
		object = GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id);
	else
		object = CCameraManager::GetInstance()->GetMainCamera();
	if (object) {
		m_mappedGameScene->m_lights[0].m_position = Vector3::Add(object->GetPosition(), XMFLOAT3(-25.f, 0, -25.f));
		m_mappedGameScene->m_lights[1].m_position = object->GetPosition();
	}
	if (KEY_TAP(KEY::Q))
		if (m_miniMap)m_miniMap = false;
		else m_miniMap = true;

	Sea_level_rise(1); 
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
					if (object && (object->IsActive()) && (!object->IsDeleted()))
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
		if (object && (object->m_name == "Ocean" || object->IsVisible(camera))) {
			if ((object->IsActive()) && (!object->IsDeleted()))
			{
				object->Render(camera);
			}
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

	for (auto& object : m_objects[(int)GROUP_TYPE::UI])
	{
		object.second->Render(camera);
	}

	for (auto& object : GetViewList(0))
	{
		if (object && object->IsVisible(camera)) 
		{
			object->RenderBilboard(camera);
		}
	}

	if(CSceneManager::GetInstance()->GetCurrentScene()->GetName() == "GameScene") RenderImGui();
	
}

void CGameScene::RenderImGui()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	CPlayer* player = reinterpret_cast<CPlayer*>(GetIDObject(GROUP_TYPE::PLAYER, framework->my_id));
	if (player->myItems.size() == 0) return;
	framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

	DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	int rows = 3;
	int cols = 6;
	static ImVec2 select = { -1, -1 };
	static string selectStr;
	const float itemSize = framework->GetResolution().x * 3 / 5 / 8;
	static bool inven = false;
	static bool option = false;
	if (KEY_TAP(KEY::E)) inven = !inven;
	if (KEY_TAP(KEY::ESC)) {
		if (inven)
			inven = false;
		else
			option = !option;
	}

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	if (inven) {
		static bool hovered[3][6] = { false };
		const ImVec4 hoverColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 외곽선 색상 (빨강)
		ImVec2 resolution = ImVec2(framework->GetResolution().x, framework->GetResolution().y); // 윈도우 해상도

		ImVec2 windowSize(framework->GetResolution().x * 3 / 5, framework->GetResolution().y * 2 / 3);
		ImVec2 windowPos((framework->GetResolution().x - windowSize.x) / 2, (framework->GetResolution().y - windowSize.y) / 2);

		// 배경색 설정 (검은색 반투명)
		ImU32 backgroundColor = IM_COL32(0, 0, 0, 128);

		// 배경 그리기
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), resolution, backgroundColor);
		DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::Begin("Inventory", nullptr, window_flags);

		// 아이템 크기 계산
		const float itemSize = windowSize.x / 8;
		const float spacing = itemSize / 7;
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

				if (j > 0) // 첫 번째 아이템이 아닌 경우에만 간격 추가
					ImGui::SameLine(0.0f, spacing);

					// 외곽선 색상 설정
					ImVec4 borderColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
					if (hovered[i][j]) borderColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

					ImGui::GetFont()->Scale = 1.0f;
					ImGui::PushFont(ImGui::GetFont());
					ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

					ImGui::BeginChildFrame(ImGui::GetID((void*)(intptr_t)(i * cols + j)), ImVec2(itemSize, itemSize));
					{
						if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
							if (ImGui::GetIO().MouseClicked[0] && !player->myItems[i * cols + j].m_name.empty()) {
								select.x = i; select.y = j;
							}
							else if (KEY_TAP(KEY::RBUTTON) && !player->myItems[i * cols + j].m_name.empty()) {
								player->myItems[i * cols + j].m_capacity -= 1;
								for (int k = 0; k < 18; ++k) {
									if (player->myItems[k].m_name.empty()) {
										player->myItems[k].m_name = player->myItems[i * cols + j].m_name;
										player->myItems[k].m_capacity = 1;
										select.x = (int)k / cols; select.y = (int)k % cols;
										break;
									}
								}
								if (player->myItems[i * cols + j].m_capacity == 0) {
									player->myItems[i * cols + j].m_name.clear();
									player->myItems[i * cols + j].m_capacity = 0;
								}
							}
							else if (KEY_AWAY(KEY::LBUTTON) || (KEY_AWAY(KEY::RBUTTON))) {
								if (select.x != -1 || select.y != -1) {
									auto str = player->myItems[i * cols + j];
									auto item = player->myItems[select.x * cols + select.y];
									if ((select.x != i || select.y != j) && str.m_name == item.m_name) {
										player->myItems[i * cols + j].m_capacity += player->myItems[select.x * cols + select.y].m_capacity;
										player->myItems[select.x * cols + select.y].m_name.clear();
										player->myItems[select.x * cols + select.y].m_capacity = 0;
									
									}
									else {
										player->myItems[i * cols + j] = item;
										player->myItems[select.x * cols + select.y] = str;
									}
								}
							}
							else
								hovered[i][j] = true;
						}
						else
							hovered[i][j] = false;

						if (i != select.x || j != select.y) {
							if (!player->myItems[i * cols + j].m_name.empty()) {
								int item = CAssetManager::GetInstance()->m_IconTextures.count(player->myItems[i * cols + j].m_name);
								if (item != 0) {
									auto& handle = CAssetManager::GetInstance()->m_IconTextures[player->myItems[i * cols + j].m_name]->m_IconGPUHandle;
									ImGui::Image((void*)handle.ptr, ImVec2(itemSize * 2 / 3, itemSize * 2 / 3));
								}
								if(player->myItems[i * cols + j].m_capacity == 1)
									ImGui::Text("%s", player->myItems[i * cols + j].m_name.c_str());
								else
									ImGui::TextWrapped("%s x %d", player->myItems[i * cols + j].m_name.c_str(), player->myItems[i * cols + j].m_capacity);
							}
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

		if (select.x != -1 && select.y != -1)
		{
			if (KEY_AWAY(KEY::LBUTTON) || KEY_AWAY(KEY::RBUTTON)) {
				if ((CURSOR.x <= windowPos.x || windowPos.x + windowSize.x <= CURSOR.x) ||
					(CURSOR.y <= windowPos.y || windowPos.x + windowSize.y <= CURSOR.y)) {
					cout << "Drop Items" << endl;

					CS_DROPED_ITEM send_packet;
					send_packet.m_size = sizeof(send_packet);
					send_packet.m_type = P_CS_DROPED_ITEM;
					send_packet.m_capacity = player->myItems[select.x * cols + select.y].m_capacity;
					strcpy_s(send_packet.m_itemName, (player->myItems[select.x * cols + select.y]).m_name.c_str());
					PacketQueue::AddSendPacket(&send_packet);

					player->myItems[select.x * cols + select.y].m_name.clear();
				}
				select.x = -1; select.y = -1;
			}
			else {
				ImVec2 mousepos = ImGui::GetMousePos();
				mousepos.x += 5;
				mousepos.y += 5;
				//ImGui::SetCursorPos(ImVec2(mousepos.x - windowPos.x - itemSize / 2,
				//	mousepos.y - windowPos.y - itemSize / 2));
				ImGui::SetNextWindowSize(ImVec2(itemSize, itemSize), ImGuiCond_Always);
				ImGui::SetNextWindowPos(mousepos, ImGuiCond_Always);
				ImGui::Begin("cursor", nullptr, window_flags);
				{
					if (!player->myItems[select.x * cols + select.y].m_name.empty()) {
						int item = CAssetManager::GetInstance()->m_IconTextures.count(player->myItems[select.x * cols + select.y].m_name);
						if (item != 0) {
							auto& handle = CAssetManager::GetInstance()->m_IconTextures[player->myItems[select.x * cols + select.y].m_name]->m_IconGPUHandle;
							ImGui::Image((void*)handle.ptr, ImVec2(itemSize * 2 / 3, itemSize * 2 / 3));
						}
						if (player->myItems[select.x * cols + select.y].m_capacity == 1)
							ImGui::Text("%s", player->myItems[select.x * cols + select.y].m_name.c_str());
						else
							ImGui::TextWrapped("%s x %d", player->myItems[select.x * cols + select.y].m_name.c_str(), player->myItems[select.x * cols + select.y].m_capacity);
					}
					ImGui::End();
				}
			}
		}

	}

		ImGuiRenderMiniMap();

	rows = 3;
	static bool selected[3] = { false };
	const ImVec4 hoverColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 외곽선 색상 (빨강)

	ImVec2 windowSize(framework->GetResolution().x * 1 / 4, framework->GetResolution().x * 1 / 4 / 3);
	ImVec2 windowPos((framework->GetResolution().x - windowSize.x) / 2, (framework->GetResolution().y - windowSize.y));

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::Begin("Window", nullptr, window_flags);


	// 아이템 크기 계산
	const float InventoryItemSize = windowSize.y * 5 / 6;

	ImGui::SetCursorPosX(InventoryItemSize / 5);
	for (int i = 0; i < rows; ++i)
	{
		ImGui::PushID(i + 1000);

		// 외곽선 색상 설정
		ImVec4 borderColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		if (selected[i]) borderColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		ImGui::GetFont()->Scale = 1.0f;
		ImGui::PushFont(ImGui::GetFont());
		ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		ImGui::BeginChildFrame(ImGui::GetID((void*)(intptr_t)(i + 1000)), ImVec2(InventoryItemSize, InventoryItemSize));
		{
			ImGui::Text("%d", i + 1);
			if (!player->myItems[i].m_name.empty()) {
				int item = CAssetManager::GetInstance()->m_IconTextures.count(player->myItems[i].m_name);
				if (item != 0) {
					ImGui::SameLine();
					auto& handle = CAssetManager::GetInstance()->m_IconTextures[player->myItems[i].m_name]->m_IconGPUHandle;
					ImGui::Image((void*)handle.ptr, ImVec2(InventoryItemSize * 2 / 3, InventoryItemSize * 2 / 3));
				}
				if (player->myItems[i].m_capacity == 1)
					ImGui::Text("%s", player->myItems[i].m_name.c_str());
				else
					ImGui::TextWrapped("%s x %d", player->myItems[i].m_name.c_str(), player->myItems[i].m_capacity);
			}
			ImGui::EndChildFrame();
		}
		ImGui::SameLine();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::PopID();
	}
	ImGui::End();

	windowSize.x = framework->GetResolution().x / 3;
	windowSize.y = framework->GetResolution().y / 15;
	windowPos.x = framework->GetResolution().x / 2 - windowSize.x/2;
	windowPos.y = 10;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::GetFont()->Scale = 3.0f;
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 50, 50, 255)); // RGBA
	ImGui::PushFont(ImGui::GetFont());

	ImGui::Begin("Phase Timer", nullptr, window_flags);
	{
		ImGui::Text("Phase %d | Next Phase : %ds", CTimeManager::GetInstance()->m_phase, (int)CTimeManager::GetInstance()->m_lastTime);
	}

	ImGui::PopFont();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();

	windowSize.x = framework->GetResolution().x / 3;
	windowSize.y = framework->GetResolution().y / 6;
	windowPos.x = 10;
	windowPos.y = framework->GetResolution().y - windowSize.y - 10;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

	{
		ImGui::Begin("State", nullptr, window_flags);
		ImGui::GetFont()->Scale = 3.0f;
		ImGui::PushFont(ImGui::GetFont());
		ImGui::Text(player->m_name.c_str());
		ImGui::PopFont();

		ImGui::GetFont()->Scale = 3.0f;
		ImGui::PushFont(ImGui::GetFont());
		ImGui::Text("HP: %d", clamp(player->GetHealth(), 0, 100));

		// Health Bar
		float rate = (float)player->GetHealth() / 100;
		ImVec2 size = ImVec2(windowSize.x * 2/3, windowSize.y / 4);
		ImVec2 top_left = ImVec2(windowPos.x + 200, windowPos.y + 50);
		ImVec2 bottom_right = ImVec2(top_left.x + size.x, top_left.y + size.y);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(top_left, bottom_right, IM_COL32(255, 255, 255, 180));

		bottom_right = ImVec2(top_left.x + size.x * rate, top_left.y + size.y);
		draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(top_left, bottom_right, IM_COL32(255, 50, 50, 255));

		auto& handle = CAssetManager::GetInstance()->m_IconTextures["election_ticket"]->m_IconGPUHandle;
		ImGui::Image((void*)handle.ptr, ImVec2(windowSize.y / 4, windowSize.y / 4));
		ImGui::SameLine(); 
		ImGui::Text("X %d", player->m_tickets);
		
		ImGui::PopFont();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();
	}

	
	windowSize.x = framework->GetResolution().x / 5;
	windowSize.y = framework->GetResolution().y / 2;
	windowPos.x = framework->GetResolution().x - windowSize.x;
	windowPos.y = framework->GetResolution().y * 9 / 10;

	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));

	const float imagesize = framework->GetResolution().y / 15;
	{
		ImGui::Begin("Key Info", nullptr, window_flags);

		if (player->m_Weapon == WEAPON_TYPE::PISTOL) {
			auto& gunhandle = CAssetManager::GetInstance()->m_IconTextures["gun"]->m_IconGPUHandle;
			ImGui::Image((void*)gunhandle.ptr, ImVec2(imagesize, imagesize));
			ImGui::SameLine();
			int bullets = 0;
			for (auto items : player->myItems) {
				if(items.m_name == "bullets")
					bullets += items.m_capacity;
			}
			ImGui::GetFont()->Scale = 2.0f;
			ImGui::PushFont(ImGui::GetFont());
			ImGui::Text("Bullets : %d / %d", player->m_bullets, bullets);
			ImGui::PopFont();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::End();
	}

	if (!inven) {
		if (framework->m_players > 1)
		{
			// ImPlot 창 시작
			static ImPlotPieChartFlags flags = true;
			ImGui::SetNextItemWidth(250);

			CSelectScene* selectscene = reinterpret_cast<CSelectScene*>(CSceneManager::GetInstance()->GetScene(SCENE_TYPE::SELECT));

			static const char* labels[3];
			static int data[3];

			int j = 0;
			for (int i = 0; i < 3; ++i) {
				if (selectscene->m_selected_id[i] == -1) continue;
				CObject* object = GetIDObject(GROUP_TYPE::PLAYER, selectscene->m_selected_id[i]);
				if (object == NULL) { j += 1; continue; }
				labels[j] = object->GetName().c_str();
				data[j++] = reinterpret_cast<CPlayer*>(object)->m_tickets;
			}

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
			ImGui::GetFont()->Scale = 1.0f;
			ImGui::PushFont(ImGui::GetFont());

			ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
			ImGui::Begin("Chard", nullptr, window_flags);
			{
				ImGui::Text("The Approval Rating");
				ImPlotStyle& style = ImPlot::GetStyle();
				style.Colors[ImPlotCol_PlotBg] = ImVec4(1, 1, 1, 1);
				style.Colors[ImPlotCol_FrameBg] = ImVec4(0, 0, 0, 0);
				style.Colors[ImPlotCol_PlotBorder] = ImVec4(0, 0, 0, 1);

				ImGui::SetCursorPos(ImVec2(0, 15));

				ImU32 origin_colors[3] = {
				IM_COL32(52, 86, 237, 255),    // Blue
				IM_COL32(217, 46, 46, 255),    // Red
				IM_COL32(237, 227, 28, 255)   // Yellow
				};
				ImU32 colors[3];
				int j = 0;
				for (int i = 0; i < 3; ++i) {
					if (selectscene->m_selected_id[i] != -1)
						colors[j++] = origin_colors[i];
				}
				static ImPlotColormap Liars = ImPlot::AddColormap("Liars", colors, framework->m_players);
				ImPlot::PushColormap(Liars);
				if (ImPlot::BeginPlot("##Pie2", ImVec2(250, 250), ImPlotFlags_Equal | ImPlotFlags_NoMouseText)) {
					ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
					ImPlot::SetupAxesLimits(0, 1, 0, 1);

					ImPlot::PlotPieChart(labels, data, framework->m_players, 0.5, 0.5, 0.4, "%.0f", 0, flags);

					// 가운데 구멍을 채우기 위해 원을 그립니다
					ImDrawList* draw_list = ImPlot::GetPlotDrawList();
					ImVec2 center = ImPlot::PlotToPixels(ImPlotPoint(0.5f, 0.5f));
					float radius = ImPlot::GetCurrentPlot()->PlotRect.GetWidth() * 0.4f * 0.5f;
					draw_list->AddCircleFilled(center, radius * 0.7f, IM_COL32(255, 255, 255, 255));

					ImPlot::EndPlot();
				}
			}
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::End();
		}

		if (option || player->m_dead) {
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
			ImGui::GetFont()->Scale = 2.0f;
			ImGui::PushFont(ImGui::GetFont());

			ImGui::SetNextWindowSize(ImVec2(200, 200));
			ImGui::SetNextWindowPos(ImVec2(framework->GetResolution().x/2- 100, framework->GetResolution().y/2- 100), ImGuiCond_Always);
			ImGui::Begin("Exit", nullptr, window_flags);
			{
				if(player->m_dead) ImGui::Text("You Dead");
				if (ImGui::Button("Exit", ImVec2(200, 100))) {
					PostQuitMessage(0);
				}
			}
			ImGui::PopFont();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::End();
		}
	}

	ImGui::Render();

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());
}

void CGameScene::DrawDamageBackGroundImGui()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	// 화면 해상도 가져오기
	ImVec2 resolution = ImVec2(framework->GetResolution().x, framework->GetResolution().y);

	ImU32 backgroundColor = IM_COL32(0, 0, 0, 128); // RGBA (0, 0, 0, 128) - 반투명 검은색
	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), resolution, backgroundColor);

	// 윈도우 플래그 설정
	DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

}

#include <cmath> // for sin function

void CGameScene::Sea_level_rise(float a)
{
	if (m_Ocean == nullptr) {
		//std::cout << "바다 없음!" << std::endl;
		return;
	}

	CTransform* transform = reinterpret_cast<CTransform*>(m_Ocean->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 nowPos = transform->GetPosition();
	if (m_fOceanHeight <= nowPos.y) return;

	transform->SetPosition(XMFLOAT3(nowPos.x, nowPos.y + 10 * DT, nowPos.z));
}

void CGameScene::ImGuiRenderMiniMap()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	// 화면 해상도 가져오기
	ImVec2 resolution = ImVec2(framework->GetResolution().x, framework->GetResolution().y);

	// 윈도우 사이즈 및 위치 설정
	ImVec2 windowSize = m_miniMap ? ImVec2(600, 600) : ImVec2(200, 200);
	ImVec2 windowPos = m_miniMap ? ImVec2((resolution.x - windowSize.x) / 2, (resolution.y - windowSize.y) / 2)
		: ImVec2(resolution.x - windowSize.x, 0);

	// 배경색 설정 (검은색 반투명)
	if (m_miniMap) {
		ImU32 backgroundColor = IM_COL32(0, 0, 0, 128); // RGBA (0, 0, 0, 128) - 반투명 검은색
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), resolution, backgroundColor);
	}

	// 윈도우 플래그 설정
	DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

	// 다음 윈도우의 크기와 위치 설정
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

	// 윈도우 시작
	ImGui::Begin("Mini Map Window", nullptr, window_flags);

	// 미니맵 이미지 렌더링
	auto assetManager = CAssetManager::GetInstance();
	auto& iconTextures = assetManager->m_IconTextures;
	auto& minimapTexture = iconTextures["MinimapIcons/MiniMap"];
	auto& minimapHandle = minimapTexture->m_IconGPUHandle;
	if(m_miniMap)ImGui::Image((void*)minimapHandle.ptr, ImVec2(windowSize.x, windowSize.y));

	// 플레이어 객체 가져오기
	CPlayer* player = reinterpret_cast<CPlayer*>(GetIDObject(GROUP_TYPE::PLAYER, framework->my_id));
	CObject* focusObject = nullptr;

	// 플레이어 ID로 객체 가져오기
	focusObject = player;


	// 확대 여부에 따른 중심점 설정
	ImVec2 centerPos;
	XMFLOAT3 focusPos3D = focusObject->GetPosition();
	float scaleFactor = m_miniMap ? 1.0f : 3.0f;

	centerPos = ImVec2(windowPos.x + windowSize.x / 2, windowPos.y + windowSize.y / 2);

	for (int i = static_cast<int>(GROUP_TYPE::STRUCTURE); i <= static_cast<int>(GROUP_TYPE::GROUND_ITEM); ++i) {
		const unordered_map<int, CObject*>& objects = GetGroupObject(static_cast<GROUP_TYPE>(i));

		for (const auto& objectsPair : objects) {
			CObject* object = objectsPair.second;
			XMFLOAT3 npcPos3D = object->GetPosition();

			// 확대 모드일 때 중심 객체 기준 좌표 변환
			float mapX, mapY;
			if (m_miniMap) {
				mapX = (npcPos3D.x - 200.f) / 600.0f * windowSize.x;
				mapY = (npcPos3D.z - 200.f) / 600.0f * windowSize.y; // Y축 반전
			}
			else {
				mapX = (npcPos3D.x - focusPos3D.x) / 600.0f * windowSize.x * scaleFactor;
				mapY = (npcPos3D.z - focusPos3D.z) / 600.0f * windowSize.y * scaleFactor; // Y축 반전
			}

			// 3D 좌표를 2D 미니맵 좌표로 변환 (0, 0을 중심으로 -45도 회전)
			float rotatedX = (mapX + mapY) * 0.7071f; // cos(-45°) = 0.7071, sin(-45°) = -0.7071
			float rotatedY = (mapY - mapX) * 0.7071f;

			ImVec2 npcPos = ImVec2(centerPos.x + rotatedX, centerPos.y - rotatedY); // Y축 반전 후 적용
			D3D12_GPU_DESCRIPTOR_HANDLE npcHandle;
			ImVec2 npcSize;

			switch (static_cast<GROUP_TYPE>(i)) {
			case GROUP_TYPE::STRUCTURE:
				continue;
			case GROUP_TYPE::PLAYER:
				npcSize = ImVec2(20, 20);
				npcHandle = CAssetManager::GetInstance()->m_IconTextures["MinimapIcons/RadPlayer"]->m_IconGPUHandle;
				break;
			case GROUP_TYPE::NPC:
				npcSize = ImVec2(15, 25);
				npcHandle = CAssetManager::GetInstance()->m_IconTextures["MinimapIcons/GrayNPC"]->m_IconGPUHandle;
				break;
			case GROUP_TYPE::MONSTER:
				//npcSize = ImVec2(20, 20);
				//npcHandle = CAssetManager::GetInstance()->m_IconTextures["MinimapIcons/MonsterIcon"]->m_IconGPUHandle;
				break;
			case GROUP_TYPE::GROUND_ITEM:
				npcSize = ImVec2(20, 20);
				npcHandle = CAssetManager::GetInstance()->m_IconTextures["MinimapIcons/ItemIcon"]->m_IconGPUHandle;
				break;
			default:
				continue;
			}

			ImGui::SetCursorPos(ImVec2(npcPos.x - windowPos.x - npcSize.x / 2, npcPos.y - windowPos.y - npcSize.y / 2));
			ImGui::Image((void*)npcHandle.ptr, npcSize);
		}
	}

	// 윈도우 끝
	ImGui::End();
}

void CGameScene::SetLightVersion(int value)
{
	// 낮 버전 설정
	if (value % 2 != 0)
	{
		m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f); // 밝고 따뜻한 색상
		m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(1.0f, 1.0f, 0.9f, 1.0f); // 강하고 선명한 흰색
		m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.9f, 0.9f, 0.8f, 1.0f); // 높은 강도의 흰색
		m_mappedGameScene->m_lights[1].m_isActive = true;
		m_mappedGameScene->m_lights[1].m_shadowMapping = false;
		m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
		m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 100.0f, 0.0f); // 태양이 하늘에 있는 것처럼 위에서 아래로 비추는 방향
		m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 0.0f));
		m_mappedGameScene->m_lights[1].m_range = 1000.f;
	}
	// 밤 버전 설정
	else
	{
		m_mappedGameScene->m_lights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.2f, 1.0f); // 어둡고 차가운 색상
		m_mappedGameScene->m_lights[1].m_xmf4Diffuse = XMFLOAT4(0.2f, 0.2f, 0.4f, 1.0f); // 약한 푸른빛
		m_mappedGameScene->m_lights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.5f, 1.0f); // 낮은 강도의 흰색 또는 푸른색
		m_mappedGameScene->m_lights[1].m_isActive = true;
		m_mappedGameScene->m_lights[1].m_shadowMapping = false;
		m_mappedGameScene->m_lights[1].m_type = static_cast<int>(LIGHT_TYPE::DIRECTIONAL);
		m_mappedGameScene->m_lights[1].m_position = XMFLOAT3(0.0f, 10.0f, -10.0f); // 달빛이나 도시 불빛을 반영하여 다양한 방향
		m_mappedGameScene->m_lights[1].m_direction = Vector3::Normalize(XMFLOAT3(0.0f, -1.0f, 0.5f));
		m_mappedGameScene->m_lights[1].m_range = 100.f;
	}
}