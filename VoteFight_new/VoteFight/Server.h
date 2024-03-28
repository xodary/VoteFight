#pragma once
#include "Protocol.h"

struct SOCKET_INFO
{
	UINT			m_ID{};
	SOCKET			m_Socket{};
	SOCKADDR_IN		m_SocketAddress{};
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

class SERVER{
public:
	static SOCKET_INFO		m_SocketInfo;
	
public:

	static void ConnectServer();
};