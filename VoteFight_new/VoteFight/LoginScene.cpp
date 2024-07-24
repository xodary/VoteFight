#include "pch.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "GameFramework.h"
#include "LoginScene.h"
#include "ServerManager.h"
#include "../Packet.h"
#include "ImaysNet/PacketQueue.h"
#include "SoundManager.h"

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
}

void CLoginScene::InitUI()
{
}

void CLoginScene::Update()
{
    if (!startSong) // �� �Ŵ����� ���� �Ŵ��� ���� ���� Enter�ؼ����� �𸣰ھ ���߿� ����
    {
        startSong = true;
        CSoundManager::GetInstance()->Play(SOUND_TYPE::TITLE_BGM, 1.0f, false); 
    }
}

void CLoginScene::PreRender()
{
}

void CLoginScene::Render() {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    CGameFramework* framework = CGameFramework::GetInstance();
    framework->GetGraphicsCommandList()->SetDescriptorHeaps(1, &framework->m_GUISrvDescHeap);

    // ȭ�� ũ�⸦ ���ͼ� ������ ũ�� ����
    ImVec2 window_size{ io.DisplaySize.x / 2.0f, io.DisplaySize.y / 3.0f };

    // ���콺 ��ġ�� �����ϴ� �κ��� ������ ����
    POINT mousePoint;
    GetCursorPos(&mousePoint);
    ScreenToClient(framework->GetHwnd(), &mousePoint);

    io.MousePos.x = mousePoint.x * (framework->GetResolution().x / io.DisplaySize.x);
    io.MousePos.y = mousePoint.y * (framework->GetResolution().y / io.DisplaySize.y);

    // ������ ��ġ ����
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = ImVec2(io.DisplaySize.x - window_size.x - 600,  // ������ ������ 600 �ȼ� ������ ��ġ
        main_viewport->WorkPos.y + (io.DisplaySize.y - window_size.y) / 2.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);

    DWORD window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, window_size);
    ImGui::GetFont()->Scale = 3.0f;
    ImGui::PushFont(ImGui::GetFont());
    ImGui::Begin("Custom Sized Window", nullptr, window_flags);
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
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), framework->GetGraphicsCommandList());
}




CLoginScene::~CLoginScene()
{
    //ReleaseShaderVariables();
}
