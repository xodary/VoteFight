#include "pch.h"
#include "SkyboxShader.h"

CSkyboxShader::CSkyboxShader()
{
}

CSkyboxShader::~CSkyboxShader()
{
}

D3D12_INPUT_LAYOUT_DESC CSkyboxShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 1;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return d3d12InputLayoutDesc;
}

D3D12_BLEND_DESC CSkyboxShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC d3d12BlendDesc = CShader::CreateBlendState(stateNum);

	d3d12BlendDesc.AlphaToCoverageEnable = true;
	d3d12BlendDesc.RenderTarget[0].BlendEnable = true;
	d3d12BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3d12BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	return d3d12BlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CSkyboxShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC d3d12DepthStencilDesc = CShader::CreateDepthStencilState(stateNum);

	d3d12DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3d12DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	return d3d12DepthStencilDesc;
}

D3D12_SHADER_BYTECODE CSkyboxShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "VS_SkyBox", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CSkyboxShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "PS_SkyBox", "ps_5_1", d3d12ShaderBlob);
}

//D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST