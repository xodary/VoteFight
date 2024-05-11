#pragma once
#include "Shader.h"

class CAnisoMipmapShader : public CComputeShader
{
	friend class CAssetManager;

	struct CB_ANISO_MIPMAP
	{
		int MipDimension;
		int MipLevel;
	};

private:
	CAnisoMipmapShader();

	Texture3D*									mVCTAnisoMipmappinPrepare3DRTs[6];

	ID3D12RootSignature*						m_rootSignature[2];

	ComPtr<ID3D12Resource>						m_AnisoMipmapBufferPrepare;
	CB_ANISO_MIPMAP*							m_AnisoMipmapMappedDataPrepare;
	DescriptorHandle							m_cpuAnisoMipmapPrepare;

	vector<ComPtr<ID3D12Resource>>				m_AnisoMipmapBuffers;
	vector<CB_ANISO_MIPMAP*>					m_AnisoMipmapMappedDatas;
	vector<DescriptorHandle>					m_cpuAnisoMipmaps;


public:
	virtual ~CAnisoMipmapShader();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual ID3D12RootSignature* CreateComputeRootSignature(int stateNum);

	void Render(int stateNum);

	Texture3D* mVCTAnisoMipmappinMain3DRTs[6];

};
