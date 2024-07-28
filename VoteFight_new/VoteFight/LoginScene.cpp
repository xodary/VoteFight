#include "pch.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "GameFramework.h"
#include "LoginScene.h"
#include "ServerManager.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"
#include "CameraManager.h"
#include "Transform.h"
#include "Camera.h"
#include "TimeManager.h"
#include "SoundManager.h"
#include "AssetManager.h"
#include "Texture.h"

CLoginScene::CLoginScene()
{
    SetName("LoginScene");
}

void CLoginScene::Enter()
{
}

void CLoginScene::Exit()
{
    if(startSong)CSoundManager::GetInstance()->Stop(SOUND_TYPE::TITLE_BGM);
    startSong = false;
}

void CLoginScene::CreateShaderVariables()
{
}

void CLoginScene::UpdateShaderVariables()
{
}

void CLoginScene::ReleaseShaderVariables()
{
}

void CLoginScene::Init()
{
   
    CCameraManager::GetInstance()->SetGameSceneMainCamera();

    m_focus = new CObject();
    CTransform* targetTransform = static_cast<CTransform*>(m_focus->GetComponent(COMPONENT_TYPE::TRANSFORM));
    targetTransform->SetPosition(XMFLOAT3(50, 10, 50));
    CCameraManager::GetInstance()->GetMainCamera()->SetTarget(m_focus);
}

void CLoginScene::InitUI()
{
}

void CLoginScene::Update()
{
    if (!startSong)
    {
        CSoundManager::GetInstance()->Play(SOUND_TYPE::TITLE_BGM, 0.3f, false);
        startSong = true;
    }

    time -= DT * 0.03f;
    if (time < 0.0f) {
        time = 1.0f;
        m_ncamera = (m_ncamera + 1) % 4;
    }
    CTransform* transform = reinterpret_cast<CTransform*>(m_focus->GetComponent(COMPONENT_TYPE::TRANSFORM));
    transform->SetPosition(Vector3::Add(Vector3::ScalarProduct(m_fcamera[(m_ncamera + 1) % 4], 1 - time),
        Vector3::ScalarProduct(m_fcamera[m_ncamera], time)));
    CSceneManager::GetInstance()->GetGameScene()->Update();
}

void CLoginScene::PreRender()
{
    CSceneManager::GetInstance()->GetGameScene()->PreRender();
}

void CLoginScene::Render() 
{
    CSceneManager::GetInstance()->GetGameScene()->Render();

    RenderImGui();
    // RenderLogo();
}

void CLoginScene::RenderImGui()
{
    CSceneManager::GetInstance()->GetGameScene()->Render();

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    
    CGameFramework * framework = CGameFramework::GetInstance();
    framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

    XMFLOAT2 resolution = framework->GetResolution();

    ImVec2 window_size{ resolution.x * 2 / 3, resolution.y * 2 / 3 };
    ImVec2 window_pos{ resolution.x * 1 / 6, resolution.y * 1 / 6 };

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);

    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, window_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
    ImGui::GetFont()->Scale = 3.0f;

    ImGui::PushFont(ImGui::GetFont());
    ImGui::Begin("Full Screen Window", nullptr, window_flags);
    {
        ImGui::Text("Server Address :");
        ImGui::InputText("##ServrName", server_addr, IM_ARRAYSIZE(server_addr));
        const char* items[] = { "1", "2", "3" };
        static int item_current_idx = 0; // 현재 선택된 항목의 인덱스
        ImGui::Text("Players :");
        if (ImGui::BeginCombo("##Combo", items[item_current_idx]))
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::TextWrapped(login_state);
        ImGui::InputText("##ChatInput", user_name, IM_ARRAYSIZE(user_name));
        if (ImGui::Button("Host")) {
            CGameFramework::GetInstance()->m_connect_server = true;
            memset(login_state, 0, sizeof(login_state));
            strcpy(login_state, "Wait For Server Message ...");
            if (CServerManager::GetInstance()->ConnectServer(server_addr)) {
                CS_LOGIN_PACKET send_packet;
                send_packet.m_size = sizeof(CS_LOGIN_PACKET);
                send_packet.m_type = P_CS_LOGIN_PACKET;
                send_packet.m_players = item_current_idx;
                strcpy_s(send_packet.m_name, user_name);
                PacketQueue::AddSendPacket(&send_packet);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Join")) {
            CGameFramework::GetInstance()->m_connect_server = true;
            memset(login_state, 0, sizeof(login_state));
            strcpy(login_state, "Wait For Server Message ...");
            if (CServerManager::GetInstance()->ConnectServer(server_addr)) {
                CS_LOGIN_PACKET send_packet;
                send_packet.m_size = sizeof(CS_LOGIN_PACKET);
                send_packet.m_type = P_CS_LOGIN_PACKET;
                send_packet.m_players = -1;
                strcpy_s(send_packet.m_name, user_name);
                PacketQueue::AddSendPacket(&send_packet);
            }
        }
        //ImGui::SameLine();
        //if (ImGui::Button("How to Play")) {
        //    CGameFramework::GetInstance()->m_connect_server = true;
        //    auto& handle = CAssetManager::GetInstance()->m_IconTextures["HowToPlay"]->m_IconGPUHandle;
        //    ImGui::Image((void*)handle.ptr, ImVec2(resolution.x * 2 / 3, resolution.y * 2 / 3));
        //}
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());
}

CLoginScene::~CLoginScene()
{
    CSceneManager::GetInstance()->GetGameScene()->ReleaseShaderVariables();
}
