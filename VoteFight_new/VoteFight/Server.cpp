#include "pch.h"
#include "Server.h"

SOCKET_INFO SERVER::m_SocketInfo{};

void SERVER::ConnectServer()
{
	WSADATA Wsa{};

	if (WSAStartup(MAKEWORD(2, 2), &Wsa))
	{
		cout << "윈속을 초기화하지 못했습니다." << endl;
		exit(1);
	}

	m_SocketInfo.m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_SocketInfo.m_Socket == INVALID_SOCKET)
	{
		Server::ErrorQuit("socket()");
	}

	m_SocketInfo.m_SocketAddress.sin_family = AF_INET;
	m_SocketInfo.m_SocketAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	m_SocketInfo.m_SocketAddress.sin_port = htons(SERVER_PORT);

	int ReturnValue{ connect(m_SocketInfo.m_Socket, (SOCKADDR*)&m_SocketInfo.m_SocketAddress, sizeof(m_SocketInfo.m_SocketAddress)) };

	if (ReturnValue == SOCKET_ERROR)
	{
		Server::ErrorQuit("connect()");
	}

	// 플레이어 아이디를 수신한다.
	ReturnValue = recv(m_SocketInfo.m_Socket, (char*)&m_SocketInfo.m_ID, sizeof(UINT), MSG_WAITALL);

	if (ReturnValue == SOCKET_ERROR)
	{
		Server::ErrorDisplay("recv()");
	}

}