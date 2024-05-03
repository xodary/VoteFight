#pragma once
#include "Asset.h"

class CTexture : public CAsset
{
	friend class CAssetManager;

protected:
	TEXTURE_TYPE			    m_type;

	ComPtr<ID3D12Resource>		m_d3d12Texture;
	ComPtr<ID3D12Resource>		m_d3d12UploadBuffer;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3d12GpuDescriptorHandle;

public:
	CTexture();

	virtual ~CTexture();

	TEXTURE_TYPE GetType();

	ID3D12Resource* GetTexture();

	void SetGpuDescriptorHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& D3D12GpuDescriptorHandle);
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuDescriptorHandle();

	void Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, TEXTURE_TYPE textureType);
	void Load(ifstream& in);
	void Load(const string& fileName, TEXTURE_TYPE textureType);

	virtual void UpdateShaderVariable();

	virtual void ReleaseUploadBuffers();
};
