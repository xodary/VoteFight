#include "pch.h"
#include "TerrainShader.h"

CTerrainShader::CTerrainShader()
{
}

CTerrainShader::~CTerrainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 2;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return d3d12InputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "VS_Terrain", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "PS_Terrain", "ps_5_1", d3d12ShaderBlob);
}
