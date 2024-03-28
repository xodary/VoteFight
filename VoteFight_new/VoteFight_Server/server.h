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
	static MSG_TYPE					m_MsgType;           // 서버 -> 클라이어 메세지 유형
	SERVER_TO_CLIENT_DATA	m_SendedPacketData;  // 서버에서 클라로 보내는 패킷 데이터


private:
	// 클라이언트를 수용하기 위한 대기 소켓
	SOCKET						m_ListenSocket{};

	// 서버의 소켓 주소 구조체
	SOCKADDR_IN					m_SocketAddress{};

	// 접속한 클라이언트들의 소켓 정보를 담고 있는 배열
	SOCKET_INFO					m_ClientSocketInfos[MAX_CLIENT_CAPACITY]{};

	HANDLE						m_MainSyncEvents[2]{};
	HANDLE						m_ClientSyncEvents[MAX_CLIENT_CAPACITY]{};

	// 가장 최근에 접속한 클라이언트의 인덱스
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