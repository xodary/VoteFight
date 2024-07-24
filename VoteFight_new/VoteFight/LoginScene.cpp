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

CLoginScene::CLoginScene()
{
    SetName("LoginScene");
}

void CLoginScene::Enter()
{
}

void CLoginScene::Exit()
{
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
    time -= DT * 0.03f;
    if (time < 0.0f) {
        time = 1.0f;
        m_ncamera = (m_ncamera + 1) % 4;
    }
    CTransform* transform = reinterpret_cast<CTransform*>(m_focus->GetComponent(COMPONENT_TYPE::TRANSFORM));
    transform->SetPosition(Vector3::Add(Vector3::ScalarProduct(m_fcamera[(m_ncamera + 1) % 4], 1-time),
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

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    CGameFramework* framework = CGameFramework::GetInstance();
    framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

    ImVec2 window_size{ CGameFramework::GetInstance()->GetResolution().x * 2 / 3, CGameFramework::GetInstance()->GetResolution().y * 2 / 3 };
    ImVec2 window_pos{ CGameFramework::GetInstance()->GetResolution().x * 1 / 6, CGameFramework::GetInstance()->GetResolution().y * 1 / 6 };
    
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(framework->GetHwnd(), &mousePoint); 

    io.MousePos.x = mousePoint.x * (framework->GetResolution().x / io.DisplaySize.x);
    io.MousePos.y = mousePoint.y * (framework->GetResolution().y / io.DisplaySize.y);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);

    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, window_size);
    ImGui::GetFont()->Scale = 3.0f;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::Begin("Full Screen Window", nullptr, window_flags);
    {
        ImGui::Text("Server Address :");
        ImGui::InputText("##ServrName", server_addr, IM_ARRAYSIZE(server_addr));
        ImGui::Text(login_state);
        ImGui::InputText("##ChatInput", user_name, IM_ARRAYSIZE(user_name));
        ImGui::SameLine();
        if (ImGui::Button("Login")) {
            CGameFramework::GetInstance()->m_connect_server = true;
            CServerManager::GetInstance()->ConnectServer(server_addr);
            memset(login_state, 0, sizeof(login_state));
            strcpy(login_state, "Wait For Server Message ...");
            CS_LOGIN_PACKET send_packet;
            send_packet.m_size = sizeof(CS_LOGIN_PACKET);
            send_packet.m_type = P_CS_LOGIN_PACKET;
            strcpy_s(send_packet.m_name, user_name);
            PacketQueue::AddSendPacket(&send_packet);
        }
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
    //ReleaseShaderVariables();
}
