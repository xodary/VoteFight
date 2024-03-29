#pragma once

// SDKDDKVer.h를 포함하면 최고 수준의 가용성을 가진 Windows 플랫폼이 정의됩니다.
// 이전 Windows 플랫폼용 애플리케이션을 빌드하려는 경우에는 SDKDDKVer.h를 포함하기 전에
// WinSDKVer.h를 포함하고 _WIN32_WINNT를 지원하려는 플랫폼으로 설정합니다.
#include <SDKDDKVer.h>

// C 헤더 파일
#pragma comment(lib, "ws2_32")

#include <winsock2.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>
#include <shellapi.h>
#include <atlconv.h>

// C++ 헤더 파일
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include <functional>

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

namespace Vector4
{
	inline XMFLOAT4 Add(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) + XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}

	inline XMFLOAT4 Multiply(XMFLOAT4& xmf4Vector1, XMFLOAT4& xmf4Vector2)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, XMLoadFloat4(&xmf4Vector1) * XMLoadFloat4(&xmf4Vector2));
		return(xmf4Result);
	}

	inline XMFLOAT4 Multiply(float fScalar, XMFLOAT4& xmf4Vector)
	{
		XMFLOAT4 xmf4Result;
		XMStoreFloat4(&xmf4Result, fScalar * XMLoadFloat4(&xmf4Vector));
		return(xmf4Result);
	}
}
