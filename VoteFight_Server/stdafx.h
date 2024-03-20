#pragma once
#pragma comment(lib, "ws2_32")

// 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define WIN32_LEAN_AND_MEAN      
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define MAX_CLIENT_CAPACITY  3      // client 
#define SERVER_PORT          9000

// Windows header
#include <winsock2.h>

// C runtime header
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ runtime header
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <queue>

using namespace std;

// DirectX Header
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

#ifdef _UNICODE
#define tcout wcout
#define tcin  wcin
#else
#define tcout cout
#define tcin  cin
#endif

enum MSG_TYPE
{
	MSG_NONE = 0x0000,
	MSG_TITLE = 0x0001,
	MSG_INGAME = 0x0002
};

static MSG_TYPE& operator |=(MSG_TYPE& a, MSG_TYPE b)
{
	a = static_cast<MSG_TYPE>(static_cast<int>(a) | static_cast<int>(b));

	return a;
}

// 클라이언트 소켓 정보 저장 
struct SOCKET_INFO
{
	UINT		m_ID{};
	SOCKET      m_Socket{};
	SOCKADDR_IN m_SocketAddress{};

	bool	    m_Completed{};
};

struct CLIENT_TO_SERVER_DATA
{
	UINT	   m_InputMask{};
	XMFLOAT4X4 m_WorldMatrix{};
};

struct SERVER_TO_CLIENT_DATA
{
	MSG_TYPE			m_MsgType;

	XMFLOAT4X4          m_PlayerWorldMatrixes[MAX_CLIENT_CAPACITY]{};
	//ANIMATION_CLIP_TYPE m_PlayerAnimationClipTypes[MAX_CLIENT_CAPACITY]{};
};

namespace Server
{
	void ErrorQuit(const char* Msg);
	void ErrorDisplay(const char* Msg);
}