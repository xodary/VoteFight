#include "pch.h"
#include "Texture.h"
#include "GameFramework.h"
#include "AssetManager.h"

CTexture::CTexture() :
	m_type(),
	m_d3d12Texture(),
	m_d3d12UploadBuffer(),
	m_d3d12GpuDescriptorHandle()
{
}

CTexture::~CTexture()
{
}

TEXTURE_TYPE CTexture::GetType()
{
	return m_type;
}

ID3D12Resource* CTexture::GetTexture()
{
	return m_d3d12Texture.Get();
}

void CTexture::SetGpuDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& d3d12GpuDescriptorHandle)
{
	m_d3d12GpuDescriptorHandle = d3d12GpuDescriptorHandle;
}

const D3D12_GPU_DESCRIPTOR_HANDLE& CTexture::GetGpuDescriptorHandle()
{
	return m_d3d12GpuDescriptorHandle;
}

void CTexture::Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, TEXTURE_TYPE textureType)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();

	m_type = textureType;
	m_d3d12Texture = DX::CreateTextureResource(d3d12Device, Width, Height, 1, 0, D3D12ResourceStates, D3D12ResourceFlags, DxgiFormat, D3D12ClearValue);
}

void CTexture::Load(ifstream& in)
{
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Type>")
		{
			in.read(reinterpret_cast<char*>(&m_type), sizeof(int));
		}
		else if (str == "<FileName>")
		{
			File::ReadStringFromFile(in, str);
			Load(str, m_type);
		}
		else if (str == "</Texture>")
		{
			break;
		}
	}
}

void CTexture::Load(const string& fileName, TEXTURE_TYPE textureType)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Texture\\" + fileName + ".dds";

	m_name = fileName;
	m_type = textureType;
	m_d3d12Texture = DX::CreateTextureResourceFromDDSFile(d3d12Device, d3d12GraphicsCommandList, filePath, D3D12_RESOURCE_STATE_GENERIC_READ, m_d3d12UploadBuffer.GetAddressOf());
}

void CTexture::UpdateShaderVariable()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	switch (m_type)
	{
	case TEXTURE_TYPE::ALBEDO_MAP:
		d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::ALBEDO_MAP), m_d3d12GpuDescriptorHandle);
		break;
	case TEXTURE_TYPE::NORMAL_MAP:
		d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::NORMAL_MAP), m_d3d12GpuDescriptorHandle);
		break;
	case TEXTURE_TYPE::CUBE_MAP:
		d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::CUBE_MAP), m_d3d12GpuDescriptorHandle);
		break;
	case TEXTURE_TYPE::SHADOW_MAP:
		d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::SHADOW_MAP), m_d3d12GpuDescriptorHandle);
		break;
	}
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_d3d12UploadBuffer.Get() != nullptr)
	{
		m_d3d12UploadBuffer.Reset();
	}
}
