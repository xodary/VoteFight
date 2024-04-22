#pragma once

#include "./ImaysNet/targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <tchar.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

// C 헤더 파일
#include <atlconv.h>
#include <assert.h>
#include <cstdint>
#include <memory.h>
#include <stdio.h>
#include <shellapi.h>
#include <wrl.h>

// C++ 헤더 파일
#include <atomic>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std;

// DirectX 헤더 파일
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#include "D3DX12.h"
#include "DDSTextureLoader12.h"
#include <D3D11on12.h>
#include <D3Dcompiler.h>
#include <D2D1_3.h>
#include <DXGI1_4.h>
#include <DXGIDebug.h>
#include <DWrite.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

// 사용자 정의 헤더 파일
#include "define.h"
#include "udt.h"
#include "func.h"


