#pragma once
#include "Protocol.h"

struct SOCKET_INFO
{
	UINT		m_ID{};
	SOCKET      m_Socket{};
	SOCKADDR_IN m_SocketAddress{};
};

struct CLIENT_TO_SERVER_DATA
{
	MSG_TYPE			m_recvMsgType;
};

struct SERVER_TO_CLIENT_DATA
{
	MSG_TYPE			m_sendMsgType;
	XMFLOAT4X4          m_PlayerWorldMatrixes[MAX_CLIENT_CAPACITY]{};
	//ANIMATION_CLIP_TYPE m_PlayerAnimationClipTypes[MAX_CLIENT_CAPACITY]{};
};

class CServer
{
public:
	static MSG_TYPE					m_MsgType;           // ���� -> Ŭ���̾� �޼��� ����
	SERVER_TO_CLIENT_DATA	m_SendedPacketData;  // �������� Ŭ��� ������ ��Ŷ ������


private:
	// Ŭ���̾�Ʈ�� �����ϱ� ���� ��� ����
	SOCKET						m_ListenSocket{};

	// ������ ���� �ּ� ����ü
	SOCKADDR_IN					m_SocketAddress{};

	// ������ Ŭ���̾�Ʈ���� ���� ������ ��� �ִ� �迭
	SOCKET_INFO					m_ClientSocketInfos[MAX_CLIENT_CAPACITY]{};

	HANDLE						m_MainSyncEvents[2]{};
	HANDLE						m_ClientSyncEvents[MAX_CLIENT_CAPACITY]{};

	// ���� �ֱٿ� ������ Ŭ���̾�Ʈ�� �ε���
	UINT						m_RecentClientID{};

	CLIENT_TO_SERVER_DATA		m_ReceivedPacketData[MAX_CLIENT_CAPACITY]{};

public:

	CServer();
	~CServer();

	static DWORD WINAPI AcceptClient(LPVOID Arg);
	static DWORD WINAPI ProcessClient(LPVOID Arg);

	void Events();

	UINT GetClientID() const;

	bool CreatePlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress);
	void DestroyPlayer(UINT ID);

	//void BulidObj();
	void GameLoop();

};