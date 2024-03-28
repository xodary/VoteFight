#include "pch.h"
#include "server.h"
#include "Protocol.h"

CServer::CServer()
{
    WSADATA WsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &WsaData)){
        cout << "������ �ʱ�ȭ���� ���߽��ϴ�." << endl;
        exit(1);
    }

    m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_ListenSocket == INVALID_SOCKET) {
        Server::ErrorQuit("socket()");
    }

    m_SocketAddress.sin_family = AF_INET;
    m_SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_SocketAddress.sin_port = htons(SERVER_PORT);

    int returnValue{};

    returnValue = ::bind(m_ListenSocket, (SOCKADDR*)&m_SocketAddress, sizeof(m_SocketAddress));

    if (returnValue == SOCKET_ERROR) {
        Server::ErrorQuit("bind()");
    }

    returnValue = listen(m_ListenSocket, SOMAXCONN);

    if (returnValue == SOCKET_ERROR){
        Server::ErrorQuit("listen()");
    }

    Events();

    HANDLE hThread{ CreateThread(NULL, 0, AcceptClient, (LPVOID)this, 0, NULL) };

    if (hThread){
        CloseHandle(hThread);
    }

    // BuildObjects();
    // BuildLights(); -> ���� ���� update
}

CServer::~CServer()
{
    closesocket(m_ListenSocket);
    WSACleanup();
}

DWORD WINAPI CServer::AcceptClient(LPVOID Arg)
{
    CServer* Server{ (CServer*)Arg };
    SOCKET ClientSocket{};
    SOCKADDR_IN ClientAddress{};

    while (true){
        int AddressLength{ sizeof(ClientAddress) };

        ClientSocket = accept(Server->m_ListenSocket, (SOCKADDR*)&ClientAddress, &AddressLength);

        if (ClientSocket == INVALID_SOCKET){
            Server::ErrorDisplay("accept()");
            continue;
        }

        if (!Server->CreatePlayer(ClientSocket, ClientAddress)){
            closesocket(ClientSocket);
            continue;
        }

        cout << "< Ŭ���̾�Ʈ ���� > " << "IP : " << inet_ntoa(ClientAddress.sin_addr) << ", ��Ʈ ��ȣ : " << ntohs(ClientAddress.sin_port) << endl;

        HANDLE ThreadHandle{ CreateThread(NULL, 0, ProcessClient, (LPVOID)Server, 0, NULL) };

        if (ThreadHandle){
            CloseHandle(ThreadHandle);
        }
        else{
            closesocket(ClientSocket);
        }
    }

    return 0;
}

DWORD WINAPI CServer::ProcessClient(LPVOID Arg)
{
    CServer* Server{ (CServer*)Arg };
    UINT ClientID{ Server->m_RecentClientID };
    SOCKET ClientSocket{ Server->m_ClientSocketInfos[ClientID].m_Socket };

    // ���ʷ� Ŭ���̾�Ʈ���� �ʱ�ȭ�� �÷��̾��� ���̵� ����
    int ReturnValue = send(ClientSocket, (char*)&ClientID, sizeof(UINT), 0);

    tcout << "< ���� ����� Ŭ���̾�Ʈ�� ���̵� : " << ClientID << " >" << endl;

    if (ReturnValue == SOCKET_ERROR) {
        Server::ErrorDisplay("send()");
        Server->DestroyPlayer(ClientID);
        return 0;
    }

    else{
        while (true){
            WaitForSingleObject(Server->m_MainSyncEvents[0], INFINITE);
            
            ////////////// �̺κ� thread ó�� �ʿ�? //////////////
            // ���� �޼��� ������ ���� ó��
            switch (Server->m_ReceivedPacketData[ClientID].m_recvMsgType) {
            case P_CS_MOVE_PACKET:
                break;
            }

            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);
            /////////////////////////////////////////////////////
           
            switch (Server->m_SendedPacketData.m_sendMsgType) {
            case P_SC_INIT_PACKET:
                break;
            }
        }
    }

    Server->DestroyPlayer(ClientID);

    return 0;
}

UINT CServer::GetClientID() const
{
    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i){
        if (!m_ClientSocketInfos[i].m_Socket){
            return i;
        }
    }

    return UINT_MAX;
}

bool CServer::CreatePlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress)
{
    UINT ValidID{ GetClientID() };

    if (ValidID == UINT_MAX){
        return false;
    }

    m_RecentClientID = ValidID;

    m_ClientSocketInfos[ValidID].m_ID = ValidID;
    m_ClientSocketInfos[ValidID].m_Socket = Socket;
    m_ClientSocketInfos[ValidID].m_SocketAddress = SocketAddress;

    SetEvent(m_ClientSyncEvents[ValidID]);

    return true;
}

void CServer::Events()
{
    m_MainSyncEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_MainSyncEvents[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i){
        m_ClientSyncEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
}

void CServer::DestroyPlayer(UINT ID)
{
    closesocket(m_ClientSocketInfos[ID].m_Socket);

    m_ClientSocketInfos[ID].m_Socket = NULL;

    cout << "< Ŭ���̾�Ʈ ���� > " << "IP : " << inet_ntoa(m_ClientSocketInfos[ID].m_SocketAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(m_ClientSocketInfos[ID].m_SocketAddress.sin_port) << endl;
}

void CServer::GameLoop()
{
    while (true){
        ResetEvent(m_MainSyncEvents[1]);
        SetEvent(m_MainSyncEvents[0]);

        for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i){
            if (m_ClientSocketInfos[i].m_Socket) {
                WaitForSingleObject(m_ClientSyncEvents[i], 1000);
            }
        }

        // TIMER

        // Player Info update
        // calculate
        // packet update? 

        ResetEvent(m_MainSyncEvents[0]);
        SetEvent(m_MainSyncEvents[1]);
    }
}