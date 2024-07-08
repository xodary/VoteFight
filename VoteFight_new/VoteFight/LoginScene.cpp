#include "pch.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "GameFramework.h"
#include "LoginScene.h"
#include "ServerManager.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"

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
}

void CLoginScene::InitUI()
{
}

void CLoginScene::Update()
{
}

void CLoginScene::PreRender()
{
}

void CLoginScene::Render()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    CGameFramework* framework = CGameFramework::GetInstance();
    framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

    ImVec2 window_size{ (float)io.DisplaySize.x, (float)io.DisplaySize.y };
    
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(framework->GetHwnd(), &mousePoint); 

    io.MousePos.x = mousePoint.x * (framework->GetResolution().x / io.DisplaySize.x);
    io.MousePos.y = mousePoint.y * (framework->GetResolution().y / io.DisplaySize.y);

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 50, main_viewport->WorkPos.y + 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);

    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, window_size);
    ImGui::Begin("Full Screen Window", nullptr, window_flags);
    {
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("Server Address :");
        ImGui::InputText("##ServrName", server_addr, IM_ARRAYSIZE(server_addr));
        ImGui::Text(login_state);
        ImGui::InputText("##ChatInput", user_name, IM_ARRAYSIZE(user_name));
        ImGui::SameLine();
        if (ImGui::Button("Login")) {
#ifdef CONNECT_SERVER
            CServerManager::GetInstance()->ConnectServer(server_addr);
            memset(login_state, 0, sizeof(login_state));
            strcpy(login_state, "Wait For Server Message ...");
            CS_LOGIN_PACKET send_packet;
            send_packet.m_size = sizeof(CS_LOGIN_PACKET);
            send_packet.m_type = P_CS_LOGIN_PACKET;
            strcpy_s(send_packet.m_name, user_name);
            PacketQueue::AddSendPacket(&send_packet);
#else
            CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::GAME);
#endif
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());
}

CLoginScene::~CLoginScene()
{
    ReleaseShaderVariables();
}
