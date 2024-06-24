#include "pch.h"
#include "Texture.h"
#include "GameFramework.h"
#include "AssetManager.h"

CTexture::CTexture() :
	m_type(),
	m_d3d12Texture(),
	m_d3d12UploadBuffer()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	DescriptorHeapManager* descriptorManager = CGameFramework::GetInstance()->GetDescriptorHeapManager();
	m_SRVHandle = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

//void CTexture::SetGpuDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& d3d12GpuDescriptorHandle)
//{
//	m_d3d12GpuDescriptorHandle = d3d12GpuDescriptorHandle;
//}
//
//const D3D12_GPU_DESCRIPTOR_HANDLE& CTexture::GetGpuDescriptorHandle()
//{
//	return m_d3d12GpuDescriptorHandle;
//}

//void CTexture::SetCpuDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& d3d12CpuDescriptorHandle)
//{
//	m_d3d12CpuDescriptorHandle = d3d12CpuDescriptorHandle;
//}
//
//const D3D12_CPU_DESCRIPTOR_HANDLE& CTexture::GetCpuDescriptorHandle()
//{
//	return m_d3d12CpuDescriptorHandle;
//}

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
	ID3D12GraphicsCommandList* commandlist = CGameFramework::GetInstance()->GetGraphicsCommandList();
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Texture\\" + fileName + ".dds";

	m_name = fileName;
	m_type = textureType;
	m_d3d12Texture = DX::CreateTextureResourceFromDDSFile(d3d12Device, commandlist, filePath, D3D12_RESOURCE_STATE_GENERIC_READ, m_d3d12UploadBuffer.GetAddressOf());
}

void CTexture::UpdateShaderVariable()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandlist = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorManager = framework->GetDescriptorHeapManager();
	GPUDescriptorHeap* gpuDescriptorHeap = descriptorManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, m_SRVHandle);

	switch (m_type)
	{
	case TEXTURE_TYPE::ALBEDO_MAP:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::ALBEDO_MAP), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::NORMAL_MAP:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::NORMAL_MAP), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::CUBE_MAP:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::CUBE_MAP), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::SHADOW_MAP:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::SHADOW_MAP), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::G_COLOR:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::G_COLOR), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::G_NORMAL:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::G_NORMAL), srvHandle.GetGPUHandle());
		break;
	case TEXTURE_TYPE::G_WORLDPOS:
		commandlist->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::G_WORLDPOS), srvHandle.GetGPUHandle());
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

void Texture3D::Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, UINT16 depth, UINT16 mips)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();

	ComPtr<ID3D12Resource> texture = nullptr;
	CD3DX12_HEAP_PROPERTIES d3d12HeapProperties(D3D12_HEAP_TYPE_DEFAULT); //D3D12_HEAP_TYPE_READBACK
	CD3DX12_RESOURCE_DESC d3d12ResourceDesc = { D3D12_RESOURCE_DIMENSION_TEXTURE3D, 0, Width, Height, depth, mips, DxgiFormat, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12ResourceFlags };

	DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, D3D12ResourceStates, &D3D12ClearValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(texture.GetAddressOf())));

	m_d3d12Texture = texture.Get();

	if (mips > 2)
	{
		m_RTVHandles.resize(mips);
		m_UAVHandles.resize(mips);

		for (int mipLevel = 0; mipLevel < mips; mipLevel++)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DxgiFormat;
			if (depth > 0) {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				rtvDesc.Texture3D.MipSlice = mipLevel;
				rtvDesc.Texture3D.WSize = (depth >> mipLevel);
			}
			else {
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = mipLevel;
			}

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DxgiFormat;
			if (depth > 0) {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				uavDesc.Texture3D.MipSlice = mipLevel;
				uavDesc.Texture3D.WSize = (depth >> mipLevel);
			}
			else {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = mipLevel;
			}

			DescriptorHeapManager* descriptorManager = CGameFramework::GetInstance()->GetDescriptorHeapManager();
			ID3D12Device* device = CGameFramework::GetInstance()->GetDevice();
			m_RTVHandles[mipLevel] = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			device->CreateRenderTargetView(m_d3d12Texture.Get(), &rtvDesc, m_RTVHandles[mipLevel].GetCPUHandle());

			m_UAVHandles[mipLevel] = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			device->CreateUnorderedAccessView(m_d3d12Texture.Get(), nullptr, &uavDesc, m_UAVHandles[mipLevel].GetCPUHandle());

		}
	}
}
