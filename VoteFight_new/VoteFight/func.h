#pragma once

namespace Utility
{
	wstring ConvertString(const string& str);
	string ConvertString(const wstring& str);

	template <typename T>
	inline void SafeDelete(vector<T>& v)
	{
		for (int i = 0; i < v.size(); ++i)
		{
			if (v[i] != nullptr)
			{
				delete v[i];
				v[i] = nullptr;
			}
		}

		v.clear();
	}

	template <typename T, typename U>
	inline void SafeDelete(unordered_map<T, U>& um)
	{
		for (auto& p : um)
		{
			if (p.second != nullptr)
			{
				delete p.second;
				p.second = nullptr;
			}
		}

		um.clear();
	}
}

namespace DX
{
	void ThrowIfFailed(HRESULT result);

	ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, void* Data, const UINT64& bytes, D3D12_HEAP_TYPE d3d12HeapType, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer, const UINT64& width, UINT height, UINT16 depthOrArraySize, UINT16 mipLevels, D3D12_RESOURCE_DIMENSION d3d12ResourceDimension, D3D12_RESOURCE_FLAGS d3d12ResourceFlags, DXGI_FORMAT dxgiFormat);
	ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, void* Data, const UINT64& bytes, D3D12_HEAP_TYPE d3d12HeapType, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer);
	ComPtr<ID3D12Resource> CreateTexture2DResource(ID3D12Device* d3d12Device, const UINT64& width, UINT height, UINT16 depthOrArraySize, UINT16 mipLevels, D3D12_RESOURCE_STATES d3d12ResourceStates, D3D12_RESOURCE_FLAGS d3d12ResourceFlags, DXGI_FORMAT dxgiFormat, const D3D12_CLEAR_VALUE& clearValue);
	ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, const string& fileName, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer);

	void ResourceTransition(ID3D12GraphicsCommandList* d3d12GraphicsCommandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
}

namespace File
{
	void ReadStringFromFile(ifstream& in, string& str);
}

namespace Random
{
	float Range(float min, float max);
}

namespace Math
{
	bool IsZero(float f);
	bool IsEqual(float f1, float f2);

	float Distance(const XMFLOAT3& v1, const XMFLOAT3& v2);

	bool IsInTriangle(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& position);
	UINT CalculateTriangleArea(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3);

	int CounterClockWise(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3);
	bool LineIntersection(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& v4);
}

namespace Vector3
{
	bool IsZero(const XMFLOAT3& v);
	bool IsEqual(const XMFLOAT3& v1, const XMFLOAT3& v2);

	float Length(const XMFLOAT3& v);
	XMFLOAT3 Normalize(const XMFLOAT3& v);
	XMFLOAT3 Inverse(const XMFLOAT3& v);

	XMFLOAT3 Add(const XMFLOAT3& v1, const XMFLOAT3& v2);
	XMFLOAT3 Subtract(const XMFLOAT3& v1, const XMFLOAT3& v2);
	XMFLOAT3 Multiply(const XMFLOAT3& v1, const XMFLOAT3& v2);
	XMFLOAT3 Divide(const XMFLOAT3& v1, const XMFLOAT3& v2);

	XMFLOAT3 ScalarProduct(const XMFLOAT3& v, float f);
	float DotProduct(const XMFLOAT3& v1, const XMFLOAT3& v2);
	XMFLOAT3 CrossProduct(const XMFLOAT3& v1, const XMFLOAT3& v2);

	float Angle(const XMFLOAT3& v1, const XMFLOAT3& v2);

	XMFLOAT3 TransformNormal(const XMFLOAT3& v, const XMFLOAT4X4& m);
	XMFLOAT3 TransformCoord(const XMFLOAT3& v, const XMFLOAT4X4& m);
}

namespace Matrix4x4
{
	XMFLOAT4X4 Identity();
	XMFLOAT4X4 Transpose(const XMFLOAT4X4& m);
	XMFLOAT4X4 Inverse(const XMFLOAT4X4& m);

	XMFLOAT4X4 Add(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2);
	XMFLOAT4X4 Subtract(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2);
	XMFLOAT4X4 Multiply(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2);

	XMFLOAT4X4 Translation(const XMFLOAT3& position);
	XMFLOAT4X4 Rotation(const XMFLOAT3& rotation);
	XMFLOAT4X4 Rotation(const XMFLOAT3& axis, float angle);
	XMFLOAT4X4 Scale(const XMFLOAT3& scale);

	XMFLOAT4X4 OrthographicFovLH(float viewWidth, float viewHeight, float nearZ, float farZ);
	XMFLOAT4X4 PerspectiveFovLH(float fovAngleY, float aspectRatio, float nearZ, float farZ);

	XMFLOAT4X4 LookAtLH(const XMFLOAT3& position, const XMFLOAT3& focusPosition, const XMFLOAT3& worldUp);
	XMFLOAT4X4 LookToLH(const XMFLOAT3& position, const XMFLOAT3& forward, const XMFLOAT3& worldUp);
}

namespace Server
{
	void ErrorQuit(const char* Msg);
	void ErrorDisplay(const char* Msg);
}

