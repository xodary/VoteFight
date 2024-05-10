#pragma once
#include "Shader.h"

struct CB_ANISO_MIPMAP_PREPARE
{
	int MipDimension;
	int MipLevel;
};

class AnisoMipmap : public CComputeShader
{
	friend class CAssetManager;

private:
	AnisoMipmap();

	Texture3D*									mVCTAnisoMipmappinPrepare3DRTs[6];
	ID3D12RootSignature*						m_rootSignature[2];

	ComPtr<ID3D12Resource>						m_AnisoMipmapPrepareBuffer;
	CB_ANISO_MIPMAP_PREPARE*					m_AnisoMipmapPrepareMappedData;
	DescriptorHandle							m_cpuAnisoMipmapPrepare;


public:
	virtual ~AnisoMipmap();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual ID3D12RootSignature* CreateRootSignature(int stateNum);

	void UpdateShaderVariables();
	void Render(int stateNum);

};
