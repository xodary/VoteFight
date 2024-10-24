﻿#include "pch.h"
#include "GameFramework.h"
#include "ImGUI/imgui_impl_win32.h"

HINSTANCE hInst;
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR lpCmdLine,
    _In_ int nCmdShow)
{
    // 메모리 누수 체크
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    srand(unsigned int(time(nullptr)));

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // 기본 메시지 루프입니다:
    // 1. GetMessage
    // - 메세지 큐에서 메세지가 확인될 때까지 대기한다.
    // - msg.messge == WM_QUIT인 경우, false를 반환한다.(프로그램을 종료한다.)
    //
    // 2. PeekMessage
    // - 메세지 큐에 메세지의 유무와 관계 없이 대기하지 않는다.
    // - 메세지가 있을 경우 true, 없을 경우 false를 반환한다.
    MSG msg = {};

    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else // 메세지가 없는 경우 호출
        {
            CGameFramework::GetInstance()->AdvanceFrame();
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};

    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_CLASSDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"WndClass";
    wcex.hIconSm = nullptr;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(L"WndClass", L"VOTE_FIGHT", WS_OVERLAPPEDWINDOW, 0, 0, 1800, 1020, nullptr, nullptr, hInstance, nullptr);
    //HWND hWnd = CreateWindowW(L"WndClass", L"VOTE_FIGHT", WS_OVERLAPPEDWINDOW, 0, 0, 1000, 500, nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr)
    {
        return FALSE;
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    RECT rect;
    ::GetClientRect(hWnd, &rect);
    XMFLOAT2 resolution = { (float)(rect.right - rect.left), (float)(rect.bottom - rect.top) };
    CGameFramework::GetInstance()->Init(hWnd, resolution);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
