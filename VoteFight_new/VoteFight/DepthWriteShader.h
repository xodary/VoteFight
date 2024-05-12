#pragma once
#include "Shader.h"

class CDepthWriteShader : public CShader
{
	friend class CAssetManager;

private:
	CDepthWriteShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int stateNum);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

	virtual DXGI_FORMAT GetRTVFormat(int stateNum);
	virtual DXGI_FORMAT GetDSVFormat(int stateNum);

public:
	virtual ~CDepthWriteShader();
};
