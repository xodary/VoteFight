#pragma once
#include "Shader.h"

class CBilboardShader : public CShader
{
	friend class CAssetManager;

private:
	CBilboardShader();

public:
	virtual ~CBilboardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType(int stateNum);
};
