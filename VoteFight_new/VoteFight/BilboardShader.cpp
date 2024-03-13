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
	UINT inputElementCount = 5;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[2] = { "SPRITE_SIZE", 0, DXGI_FORMAT_R32G32_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[3] = { "SPRITE_INDEX", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[4] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return d3d12InputLayoutDesc;
}

D3D12_BLEND_DESC CBilboardShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC d3d12BlendDesc = CShader::CreateBlendState(stateNum);

	d3d12BlendDesc.AlphaToCoverageEnable = true;
	d3d12BlendDesc.RenderTarget[0].BlendEnable = true;
	d3d12BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3d12BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	return d3d12BlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CBilboardShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC d3d12DepthStencilDesc = CShader::CreateDepthStencilState(stateNum);

	switch (stateNum)
	{
	case 1: // SkyBox
		d3d12DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		d3d12DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	}

	return d3d12DepthStencilDesc;
}

D3D12_SHADER_BYTECODE CBilboardShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Bilboard
		return CShader::Compile("main.hlsl", "VS_Bilboard", "vs_5_1", d3d12ShaderBlob);
	case 1: // SkyBox
		return CShader::Compile("main.hlsl", "VS_SkyBox", "vs_5_1", d3d12ShaderBlob);
	}

	return CShader::CreateVertexShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CBilboardShader::CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Bilboard
		return CShader::Compile("main.hlsl", "GS_Bilboard", "gs_5_1", d3d12ShaderBlob);
	case 1: // SkyBox
		return CShader::Compile("main.hlsl", "GS_SkyBox", "gs_5_1", d3d12ShaderBlob);
	}

	return CShader::CreateGeometryShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CBilboardShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Bilboard
	case 1: // SkyBox
		return CShader::Compile("main.hlsl", "PS_Bilboard", "ps_5_1", d3d12ShaderBlob);
	}

	return CShader::CreatePixelShader(d3d12ShaderBlob, stateNum);
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE CBilboardShader::GetPrimitiveType(int stateNum)
{
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}
