#include "pch.h"
#include "DepthWriteShader.h"

CDepthWriteShader::CDepthWriteShader()
{
}

CDepthWriteShader::~CDepthWriteShader()
{
}

D3D12_INPUT_LAYOUT_DESC CDepthWriteShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	switch (stateNum)
	{
	case 0: // Standard
		inputElementCount = 1;
		d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
		d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	case 1: // With Skinning
		inputElementCount = 3;
		d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
		d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[1] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[2] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	}

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;
}

D3D12_RASTERIZER_DESC CDepthWriteShader::CreateRasterizerState(int stateNum)
{
	D3D12_RASTERIZER_DESC d3d12RasterizerDesc = CShader::CreateRasterizerState(stateNum);

	switch (stateNum)
	{
	case 0: // Standard
	case 1: // With Skinning
		d3d12RasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		//d3d12RasterizerDesc.DepthBias = 500;
		//d3d12RasterizerDesc.SlopeScaledDepthBias = 1.0f;
		break;
	}

	return d3d12RasterizerDesc;
}

D3D12_SHADER_BYTECODE CDepthWriteShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Standard
		return Compile("main.hlsl", "VS_Position", "vs_5_1", d3d12ShaderBlob);
	case 1: // With Skinning
		return Compile("main.hlsl", "VS_Position_Skinning", "vs_5_1", d3d12ShaderBlob);
	//case 2: // Debug Depth
	//	return Compile("main.hlsl", "VS_ViewPort", "vs_5_1", d3d12ShaderBlob);
	}

	return CShader::CreateVertexShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CDepthWriteShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Standard
	case 1: // With Skinning
		return Compile("main.hlsl", "PS_DepthWrite", "ps_5_1", d3d12ShaderBlob);
	//case 2: // Debug Depth
	//	return Compile("main.hlsl", "PS_ViewPort", "ps_5_1", d3d12ShaderBlob);
	}

	return CShader::CreatePixelShader(d3d12ShaderBlob, stateNum);
}

DXGI_FORMAT CDepthWriteShader::GetRTVFormat(int renderTargetNum, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Standard
	case 1: // With Skinning
		return DXGI_FORMAT_R32_FLOAT;
	}

	return CShader::GetRTVFormat(renderTargetNum, stateNum);
}

DXGI_FORMAT CDepthWriteShader::GetDSVFormat(int stateNum)
{
	switch (stateNum)
	{
	case 0: // Standard
	case 1: // With Skinning
		return DXGI_FORMAT_D32_FLOAT;
	}

	return CShader::GetDSVFormat(stateNum);
}
