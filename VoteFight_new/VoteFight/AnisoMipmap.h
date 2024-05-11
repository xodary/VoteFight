#pragma once
#include "Shader.h"

struct CB_ANISO_MIPMAP
{
	int MipDimension;
	int MipLevel;
};

class CAnisoMipmap : public CComputeShader
{
	friend class CAssetManager;

private:
	CAnisoMipmap();

	Texture3D*									mVCTAnisoMipmappinPrepare3DRTs[6];
	Texture3D*									mVCTAnisoMipmappinMain3DRTs[6];

	ID3D12RootSignature*						m_rootSignature[2];

	ComPtr<ID3D12Resource>						m_AnisoMipmapBufferPrepare;
	CB_ANISO_MIPMAP*							m_AnisoMipmapMappedDataPrepare;
	DescriptorHandle							m_cpuAnisoMipmapPrepare;

	vector<ComPtr<ID3D12Resource>>				m_AnisoMipmapBuffers;
	vector<CB_ANISO_MIPMAP*>					m_AnisoMipmapMappedDatas;
	vector<DescriptorHandle>					m_cpuAnisoMipmaps;


public:
	virtual ~CAnisoMipmap();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual ID3D12RootSignature* CreateComputeRootSignature(int stateNum);

	void Render(int stateNum);


};
