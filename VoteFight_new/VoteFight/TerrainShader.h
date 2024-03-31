#pragma once
#include "Shader.h"

class CTerrainShader : public CShader
{
	friend class CAssetManager;

private:
	CTerrainShader();

public:
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
};
