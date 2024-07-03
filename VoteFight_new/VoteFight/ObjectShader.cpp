#include "pch.h"
#include "ObjectShader.h"
#include "GameFramework.h"

CObjectShader::CObjectShader()
{
}

CObjectShader::~CObjectShader()
{
}

D3D12_INPUT_LAYOUT_DESC CObjectShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	switch (stateNum)
	{
	case 0: // Standard
		inputElementCount = 5;
		d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
		d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;

	case 1: // With Skinning
		inputElementCount = 7;
		d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
		d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		d3d12InputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	}

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;
}

D3D12_BLEND_DESC CObjectShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC d3d12BlendDesc = CShader::CreateBlendState(stateNum);

	d3d12BlendDesc.AlphaToCoverageEnable = true;
	d3d12BlendDesc.RenderTarget[0].BlendEnable = true;
	d3d12BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3d12BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	return d3d12BlendDesc;
}

D3D12_SHADER_BYTECODE CObjectShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Standard
		return Compile("main.hlsl", "VS_GBuffer", "vs_5_1", d3d12ShaderBlob);
	case 1: // With Skinning
		return Compile("main.hlsl", "VS_GBuffer_Skinning", "vs_5_1", d3d12ShaderBlob);
	}

	return CShader::CreateVertexShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CObjectShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("main.hlsl", "PS_GBuffer", "ps_5_1", d3d12ShaderBlob);
}

void CObjectShader::CreatePipelineState(int stateNum)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12RootSignature* d3d12RootSignature = CGameFramework::GetInstance()->GetRootSignature();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12GraphicsPipelineState = {};
	ComPtr<ID3DBlob> D3D12VertexShaderBlobs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	d3d12GraphicsPipelineState.pRootSignature = CreateRootSignature(stateNum);
	d3d12GraphicsPipelineState.VS = CreateVertexShader(D3D12VertexShaderBlobs[0].Get(), stateNum);
	d3d12GraphicsPipelineState.HS = CreateHullShader(D3D12VertexShaderBlobs[1].Get(), stateNum);
	d3d12GraphicsPipelineState.DS = CreateDomainShader(D3D12VertexShaderBlobs[2].Get(), stateNum);
	d3d12GraphicsPipelineState.GS = CreateGeometryShader(D3D12VertexShaderBlobs[3].Get(), stateNum);
	d3d12GraphicsPipelineState.PS = CreatePixelShader(D3D12VertexShaderBlobs[4].Get(), stateNum);
	d3d12GraphicsPipelineState.RasterizerState = CreateRasterizerState(stateNum);
	d3d12GraphicsPipelineState.BlendState = CreateBlendState(stateNum);
	d3d12GraphicsPipelineState.DepthStencilState = CreateDepthStencilState(stateNum);
	d3d12GraphicsPipelineState.StreamOutput = CreateStreamOutputState(stateNum);
	d3d12GraphicsPipelineState.InputLayout = CreateInputLayout(stateNum);
	d3d12GraphicsPipelineState.SampleMask = UINT_MAX;
	d3d12GraphicsPipelineState.PrimitiveTopologyType = GetPrimitiveType(stateNum);
	d3d12GraphicsPipelineState.NumRenderTargets = 3;
	d3d12GraphicsPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d12GraphicsPipelineState.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_SNORM;
	d3d12GraphicsPipelineState.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	d3d12GraphicsPipelineState.DSVFormat = GetDSVFormat(stateNum);
	d3d12GraphicsPipelineState.SampleDesc.Count = 1;
	d3d12GraphicsPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	DX::ThrowIfFailed(d3d12Device->CreateGraphicsPipelineState(&d3d12GraphicsPipelineState, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(m_d3d12PipelineStates[stateNum].GetAddressOf())));

	if (d3d12GraphicsPipelineState.InputLayout.pInputElementDescs != nullptr)
	{
		delete[] d3d12GraphicsPipelineState.InputLayout.pInputElementDescs;
	}
}
