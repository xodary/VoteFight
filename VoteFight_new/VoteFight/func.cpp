#include "pch.h"
#include "func.h"

namespace Utility
{
	wstring ConvertString(const string& str)
	{
		USES_CONVERSION;

		return wstring(A2W(str.c_str()));
	}

	string ConvertString(const wstring& str)
	{
		USES_CONVERSION;

		return string(W2A(str.c_str()));
	}
}

namespace DX
{
	void ThrowIfFailed(HRESULT result)
	{
		if (FAILED(result))
		{
			char debugMessage[64] = {};

			printf_s(debugMessage, _countof(debugMessage), "[Error] HRESULT of 0x % x\n", result);
			OutputDebugStringA(debugMessage);
		}
	}

	ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, void* data, const UINT64& bytes, D3D12_HEAP_TYPE d3d12HeapType, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer, const UINT64& Width, UINT Height, UINT16 DepthOrArraySize, UINT16 MipLevels, D3D12_RESOURCE_DIMENSION D3D12ResourceDimension, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DXGIFormat)
	{
		CD3DX12_HEAP_PROPERTIES d3d12HeapProperties(d3d12HeapType);
		CD3DX12_RESOURCE_DESC d3d12ResourceDesc = 
		{
			D3D12ResourceDimension,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? static_cast<UINT64>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) : 0,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? bytes : Width,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? static_cast<UINT16>(1) : Height,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? static_cast<UINT16>(1) : DepthOrArraySize,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? static_cast<UINT16>(1) : MipLevels,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? DXGI_FORMAT_UNKNOWN : DXGIFormat,
			1,
			0,
			(D3D12ResourceDimension == D3D12_RESOURCE_DIMENSION_BUFFER) ? D3D12_TEXTURE_LAYOUT_ROW_MAJOR : D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12ResourceFlags
		};
		ComPtr<ID3D12Resource> d3d12Buffer = nullptr;
		D3D12_RANGE d3d12ReadRange = {};
		UINT8* dataBuffer = nullptr;

		switch (d3d12HeapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			D3D12_RESOURCE_STATES d3d12ResourceInitialStates = { (d3d12UploadBuffer && data) ? D3D12_RESOURCE_STATE_COPY_DEST : d3d12ResourceStates };

			DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, d3d12ResourceInitialStates, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(d3d12Buffer.GetAddressOf())));

			if (d3d12UploadBuffer != nullptr && data != nullptr)
			{
				// 업로드 버퍼를 생성한다.
				d3d12HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

				d3d12ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				d3d12ResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				d3d12ResourceDesc.Width = bytes;
				d3d12ResourceDesc.Height = 1;
				d3d12ResourceDesc.DepthOrArraySize = 1;
				d3d12ResourceDesc.MipLevels = 1;
				d3d12ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
				d3d12ResourceDesc.SampleDesc.Count = 1;
				d3d12ResourceDesc.SampleDesc.Quality = 0;
				d3d12ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				d3d12ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(d3d12UploadBuffer)));

				// 업로드 버퍼를 매핑하여 데이터를 복사한다.
				DX::ThrowIfFailed((*d3d12UploadBuffer)->Map(0, &d3d12ReadRange, reinterpret_cast<void**>(&dataBuffer)));
				memcpy(dataBuffer, data, bytes);
				(*d3d12UploadBuffer)->Unmap(0, nullptr);

				// 업로드 버퍼의 내용을 디폴트 버퍼에 복사한다.
				d3d12GraphicsCommandList->CopyResource(d3d12Buffer.Get(), *d3d12UploadBuffer);

				// 리소스 상태를 변경한다.
				ResourceTransition(d3d12GraphicsCommandList, d3d12Buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, d3d12ResourceStates);
			}
			break;
		}
		case D3D12_HEAP_TYPE_UPLOAD:
		{
			d3d12ResourceStates |= D3D12_RESOURCE_STATE_GENERIC_READ;

			DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, d3d12ResourceStates, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(d3d12Buffer.GetAddressOf())));

			if (data != nullptr)
			{
				DX::ThrowIfFailed(d3d12Buffer->Map(0, &d3d12ReadRange, reinterpret_cast<void**>(&dataBuffer)));
				memcpy(dataBuffer, data, bytes);
				d3d12Buffer->Unmap(0, nullptr);
			}
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
		{
			d3d12ResourceStates |= D3D12_RESOURCE_STATE_COPY_DEST;

			DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, d3d12ResourceStates, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(d3d12Buffer.GetAddressOf())));

			if (data != nullptr)
			{
				DX::ThrowIfFailed(d3d12Buffer->Map(0, &d3d12ReadRange, reinterpret_cast<void**>(&dataBuffer)));
				memcpy(dataBuffer, data, bytes);
				d3d12Buffer->Unmap(0, nullptr);
			}
			break;
		}
		}

		return d3d12Buffer;
	}

	ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, void* data, const UINT64& bytes, D3D12_HEAP_TYPE d3d12HeapType, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer)
	{
		return CreateTextureResource(d3d12Device, d3d12GraphicsCommandList, data, bytes, d3d12HeapType, d3d12ResourceStates, d3d12UploadBuffer, bytes, 1, 1, 1, D3D12_RESOURCE_DIMENSION_BUFFER, D3D12_RESOURCE_FLAG_NONE, DXGI_FORMAT_UNKNOWN);
	}

	ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* d3d12Device, const UINT64& width, UINT height, UINT16 depthOrArraySize, UINT16 mipLevels, D3D12_RESOURCE_STATES d3d12ResourceStates, D3D12_RESOURCE_FLAGS d3d12ResourceFlags, DXGI_FORMAT dxgiFormat, const D3D12_CLEAR_VALUE& clearValue)
	{
		ComPtr<ID3D12Resource> texture = nullptr;
		CD3DX12_HEAP_PROPERTIES d3d12HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC d3d12ResourceDesc = { D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, width, height, depthOrArraySize, mipLevels, dxgiFormat, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN, d3d12ResourceFlags };


		DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, d3d12ResourceStates, &clearValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(texture.GetAddressOf())));

		return texture;
	}

	ComPtr<ID3D12Resource> CreateTextureResourceFromDDSFile(ID3D12Device* d3d12Device, ID3D12GraphicsCommandList* d3d12GraphicsCommandList, const string& FileName, D3D12_RESOURCE_STATES d3d12ResourceStates, ID3D12Resource** d3d12UploadBuffer)
	{
		ComPtr<ID3D12Resource> d3d12Texture = nullptr;
		vector<D3D12_SUBRESOURCE_DATA> subResources;
		unique_ptr<uint8_t[]> ddsData = nullptr;
		DDS_ALPHA_MODE ddsAlphaMode = DDS_ALPHA_MODE_UNKNOWN;
		bool isCubeMap = false;

		DX::ThrowIfFailed(DirectX::LoadDDSTextureFromFileEx(d3d12Device, Utility::ConvertString(FileName).c_str(), 0, D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, d3d12Texture.GetAddressOf(), ddsData, subResources, &ddsAlphaMode, &isCubeMap));

		UINT64 bytes{ GetRequiredIntermediateSize(d3d12Texture.Get(), 0, static_cast<UINT>(subResources.size())) };
		CD3DX12_HEAP_PROPERTIES D3D12HeapPropertiesDesc{ D3D12_HEAP_TYPE_UPLOAD, 1, 1 };
		CD3DX12_RESOURCE_DESC d3d12ResourceDesc{ D3D12_RESOURCE_DIMENSION_BUFFER, 0, bytes, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };

		DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&D3D12HeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(d3d12UploadBuffer)));
		UpdateSubresources(d3d12GraphicsCommandList, d3d12Texture.Get(), *d3d12UploadBuffer, 0, 0, static_cast<UINT>(subResources.size()), subResources.data());

		// 리소스 상태를 변경한다.
		ResourceTransition(d3d12GraphicsCommandList, d3d12Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, d3d12ResourceStates);

		return d3d12Texture;
	}

	void ResourceTransition(ID3D12GraphicsCommandList* d3d12GraphicsCommandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		CD3DX12_RESOURCE_BARRIER d3d12ResourceBarrier = {};

		d3d12ResourceBarrier = d3d12ResourceBarrier.Transition(resource, beforeState, afterState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAG_NONE);
		d3d12GraphicsCommandList->ResourceBarrier(1, &d3d12ResourceBarrier);
	}
}

namespace File
{
	void ReadStringFromFile(ifstream& in, string& str)
	{
		UINT len = 0;

		in.read(reinterpret_cast<char*>(&len), sizeof(BYTE));
		str.resize(len);
		in.read(reinterpret_cast<char*>(&str[0]), sizeof(BYTE) * len);
	}
}

namespace Random
{
	float Range(float min, float max)
	{
		return min + (max - min) * (static_cast<float>(rand()) / (RAND_MAX));
	}
}

namespace Math
{
	bool IsZero(float f)
	{
		return abs(f) <= EPSILON;
	}

	bool IsEqual(float f1, float f2)
	{
		return IsZero(f1 - f2);
	}

	float Distance(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return sqrt(powf(v2.x - v1.x, 2.0f) + powf(v2.y - v1.y, 2.0f) + powf(v2.z - v1.z, 2.0f));
	}

	bool IsInTriangle(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& position)
	{
		XMFLOAT3 v1p = Vector3::Subtract(position, v1);
		XMFLOAT3 v2p = Vector3::Subtract(position, v2);
		XMFLOAT3 v3p = Vector3::Subtract(position, v3);

		XMFLOAT3 v1v2 = Vector3::Subtract(v2, v1);
		XMFLOAT3 v2v3 = Vector3::Subtract(v3, v2);
		XMFLOAT3 v3v1 = Vector3::Subtract(v1, v3);

		return (Vector3::CrossProduct(v1v2, v1p).y > 0.0f) && (Vector3::CrossProduct(v2v3, v2p).y > 0.0f) && (Vector3::CrossProduct(v3v1, v3p).y > 0.0f);
	}

	UINT CalculateTriangleArea(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
	{
		return static_cast<UINT>(abs((v1.x * (v2.z - v3.z)) + (v2.x * (v3.z - v1.z)) + (v3.x * (v1.z - v2.z))));
	}

	int CounterClockWise(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
	{
		// v1과 v2를 이은 선분과 v1과 v3을 이은 선분의 기울기를 구하고, 양변 모두 분모를 없앤 후 좌변으로 이항하면 아래와 같이 CCW를 판별할 수 있는 공식이 나온다.
		float gradientDiff = (v2.x - v1.x) * (v3.z - v1.z) - (v3.x - v1.x) * (v2.z - v1.z);

		if (gradientDiff < 0.0f)
		{
			return -1;
		}
		else if (gradientDiff > 0.0f)
		{
			return 1;
		}

		return 0;
	}

	bool LineIntersection(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& v4)
	{
		return (CounterClockWise(v1, v2, v3) * CounterClockWise(v1, v2, v4) <= 0) && (CounterClockWise(v3, v4, v1) * CounterClockWise(v3, v4, v2) <= 0);
	}
}

namespace Vector3
{
	bool IsZero(const XMFLOAT3& v)
	{
		if (Math::IsZero(v.x) && Math::IsZero(v.y) && Math::IsZero(v.z))
		{
			return true;
		}

		return false;
	}

	bool IsEqual(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		if (Math::IsEqual(v1.x, v2.x) && Math::IsEqual(v1.y, v2.y) && Math::IsEqual(v1.z, v2.z))
		{
			return true;
		}

		return false;
	}

	float Length(const XMFLOAT3& v)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3Length(XMLoadFloat3(&v)));

		return result.x;
	}

	XMFLOAT3 Normalize(const XMFLOAT3& v)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&v)));

		return result;
	}

	XMFLOAT3 Inverse(const XMFLOAT3& v)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVectorScale(XMLoadFloat3(&v), -1.0f));

		return result;
	}

	XMFLOAT3 Add(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMLoadFloat3(&v1) + XMLoadFloat3(&v2));

		return result;
	}

	XMFLOAT3 Subtract(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMLoadFloat3(&v1) - XMLoadFloat3(&v2));

		return result;
	}

	XMFLOAT3 Multiply(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMLoadFloat3(&v1) * XMLoadFloat3(&v2));

		return result;
	}

	XMFLOAT3 Divide(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMLoadFloat3(&v1) / XMLoadFloat3(&v2));

		return result;
	}

	XMFLOAT3 ScalarProduct(const XMFLOAT3& v, float f)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, f * XMLoadFloat3(&v));

		return result;
	}

	float DotProduct(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3Dot(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));

		return result.x;
	}

	XMFLOAT3 CrossProduct(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));

		return result;
	}

	float Angle(const XMFLOAT3& v1, const XMFLOAT3& v2)
	{
		return XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenNormals(XMLoadFloat3(&v1), XMLoadFloat3(&v2))));
	}

	XMFLOAT3 TransformNormal(const XMFLOAT3& v, const XMFLOAT4X4& m)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3TransformNormal(XMLoadFloat3(&v), XMLoadFloat4x4(&m)));

		return result;
	}

	XMFLOAT3 TransformCoord(const XMFLOAT3& v, const XMFLOAT4X4& m)
	{
		XMFLOAT3 result = {};

		XMStoreFloat3(&result, XMVector3TransformCoord(XMLoadFloat3(&v), XMLoadFloat4x4(&m)));

		return result;
	}
}

namespace Matrix4x4
{
	XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixIdentity());

		return result;
	}

	XMFLOAT4X4 Transpose(const XMFLOAT4X4& m)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixTranspose(XMLoadFloat4x4(&m)));

		return result;
	}

	XMFLOAT4X4 Inverse(const XMFLOAT4X4& m)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m)));

		return result;
	}

	XMFLOAT4X4 Add(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMLoadFloat4x4(&m1) + XMLoadFloat4x4(&m2));

		return result;
	}

	XMFLOAT4X4 Subtract(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMLoadFloat4x4(&m1) - XMLoadFloat4x4(&m2));

		return result;
	}

	XMFLOAT4X4 Multiply(const XMFLOAT4X4& m1, const XMFLOAT4X4& m2)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMLoadFloat4x4(&m1) * XMLoadFloat4x4(&m2));

		return result;
	}

	XMFLOAT4X4 Translation(const XMFLOAT3& position)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixTranslation(position.x, position.y, position.z));

		return result;
	}

	XMFLOAT4X4 Rotation(const XMFLOAT3& rotation)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z)));

		return result;
	}

	XMFLOAT4X4 Rotation(const XMFLOAT3& axis, float angle)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle)));

		return result;
	}

	XMFLOAT4X4 Scale(const XMFLOAT3& scale)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixScaling(scale.x, scale.y, scale.z));

		return result;
	}

	XMFLOAT4X4 OrthographicFovLH(float viewWidth, float viewHeight, float nearZ, float farZ)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ));

		return result;
	}

	XMFLOAT4X4 PerspectiveFovLH(float fovAngleY, float aspectRatio, float nearZ, float farZ)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ));

		return result;
	}

	XMFLOAT4X4 LookAtLH(const XMFLOAT3& position, const XMFLOAT3& focusPosition, const XMFLOAT3& worldUp)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&focusPosition), XMLoadFloat3(&worldUp)));

		return result;
	}

	XMFLOAT4X4 LookToLH(const XMFLOAT3& position, const XMFLOAT3& forward, const XMFLOAT3& worldUp)
	{
		XMFLOAT4X4 result = {};

		XMStoreFloat4x4(&result, XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMLoadFloat3(&worldUp)));

		return result;
	}
}

namespace Server
{
	void ErrorQuit(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		MessageBox(NULL, (LPCTSTR)MsgBuffer, (LPCTSTR)Msg, MB_ICONERROR);

		LocalFree(MsgBuffer);
		exit(1);
	}

	void ErrorDisplay(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		cout << "[" << Msg << "] " << (char*)MsgBuffer;

		LocalFree(MsgBuffer);
	}
}
