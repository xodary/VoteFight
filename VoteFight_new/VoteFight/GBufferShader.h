#pragma once
#include "Shader.h"

class CGBufferShader : public CShader
{
	friend class CAssetManager;

public:
	CGBufferShader();
	virtual ~CGBufferShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

	virtual ID3D12RootSignature* CreateRootSignature(int stateNum);

	virtual void CreatePipelineState(int stateNum);

	void Render(int stateNum);
		
	CTexture*									m_GBuffer[3];
	ID3D12RootSignature*						m_rootSignature;
};

