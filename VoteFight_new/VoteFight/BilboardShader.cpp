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
	UINT inputElementCount = 1;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs};

	return d3d12InputLayoutDesc;
}

D3D12_SHADER_BYTECODE CBilboardShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "VS_Bilboard", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CBilboardShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "PS_Bilboard", "ps_5_1", d3d12ShaderBlob);
}
