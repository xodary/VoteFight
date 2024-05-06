#pragma once
#include "../VoteFight/ImaysNet/targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define Align(value, alignment) (((value + alignment - 1) / alignment) * alignment)

#include <SDKDDKVer.h>
#ifdef _WIN32
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <tchar.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

// C 헤더 파일
#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

#include <atlconv.h>
#include <assert.h>
#include <cstdint>
#include <memory.h>
#include <stdio.h>
#include <shellapi.h>
#include <tchar.h>
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
#include <functional>
#include <atlbase.h>
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

template <typename T>
inline T DivideByMultiple(T value, size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
}

#include <filesystem>
#include <shlobj.h>
#include "pix3.h"

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
    {
        if (directory_entry.is_directory())
        {
            if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
            {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty())
    {
        // TODO: Error, no PIX installation found
    }

    return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}