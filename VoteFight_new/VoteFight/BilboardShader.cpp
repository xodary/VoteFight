#include "pch.h"
#include "BilboardShader.h"

CBilboardShader::CBilboardShader()
{
}

CBilboardShader::~CBilboardShader()
{
}

D3D12_INPUT_LAYOUT_DESC CBilboardShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 2;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return d3d12InputLayoutDesc;
}

D3D12_BLEND_DESC CBilboardShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC d3d12BlendDesc = CShader::CreateBlendState(stateNum);

	d3d12BlendDesc.AlphaToCoverageEnable = false;
	d3d12BlendDesc.RenderTarget[0].BlendEnable = true;
	d3d12BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3d12BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	return d3d12BlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CBilboardShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC d3d12DepthStencilDesc = CShader::CreateDepthStencilState(stateNum);

	d3d12DepthStencilDesc.DepthEnable = false;
	d3d12DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	return d3d12DepthStencilDesc;
}

D3D12_SHADER_BYTECODE CBilboardShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "VS_Bilboard", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CBilboardShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "PS_Bilboard", "ps_5_1", d3d12ShaderBlob);
}
