#pragma once
#include "Shader.h"

class COceanShader : public CShader
{
	friend class CAssetManager;

private:
	COceanShader();

public:
	virtual ~COceanShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
};
