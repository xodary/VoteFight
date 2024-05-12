#pragma once
#include "Shader.h"
#include "Texture.h"

class CUpsampleBlur : public CComputeShader
{
	friend class CAssetManager;
public:

	CTexture* m_UpsampleBlurRT;
	ID3D12RootSignature* m_rootSignature;

	CUpsampleBlur();
	virtual ~CUpsampleBlur();

	virtual ID3D12RootSignature* CreateComputeRootSignature(int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

	void Render(int stateNum);
};

