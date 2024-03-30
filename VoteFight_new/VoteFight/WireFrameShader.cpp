#include "pch.h"
#include "WireFrameShader.h"

CWireFrameShader::CWireFrameShader()
{
}

CWireFrameShader::~CWireFrameShader()
{
}

D3D12_INPUT_LAYOUT_DESC CWireFrameShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 1;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];

	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return d3d12InputLayoutDesc;
}

D3D12_RASTERIZER_DESC CWireFrameShader::CreateRasterizerState(int stateNum)
{
	D3D12_RASTERIZER_DESC d3d12RasterizerDesc = CShader::CreateRasterizerState(stateNum);

	d3d12RasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	return d3d12RasterizerDesc;
}

D3D12_SHADER_BYTECODE CWireFrameShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "VS_Position", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CWireFrameShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return CShader::Compile("main.hlsl", "PS_Red", "ps_5_1", d3d12ShaderBlob);
}
