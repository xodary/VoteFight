#pragma once
#include "Shader.h"

class CSkyboxShader : public CShader
{
	friend class CAssetManager;

private:
	CSkyboxShader();

public:
	virtual ~CSkyboxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
};