#include "pch.h"
#include "GameFramework.h"
#include "Texture.h"
#include "GBufferShader.h"
#include "Object.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "CameraManager.h"

CGBufferShader::CGBufferShader()
{
}

CGBufferShader::~CGBufferShader()
{
}

D3D12_RASTERIZER_DESC CGBufferShader::CreateRasterizerState(int stateNum)
{
	D3D12_RASTERIZER_DESC d3d12RasterizerDesc = {};

	d3d12RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3d12RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3d12RasterizerDesc.FrontCounterClockwise = false;
	d3d12RasterizerDesc.DepthBias = 0;
	d3d12RasterizerDesc.DepthBiasClamp = 0.0f;
	d3d12RasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3d12RasterizerDesc.DepthClipEnable = true;
	d3d12RasterizerDesc.MultisampleEnable = false;
	d3d12RasterizerDesc.AntialiasedLineEnable = false;
	d3d12RasterizerDesc.ForcedSampleCount = 0;
	d3d12RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return d3d12RasterizerDesc;
}

D3D12_BLEND_DESC CGBufferShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC blendState = {};

	blendState.IndependentBlendEnable = FALSE;
	blendState.RenderTarget[0].BlendEnable = FALSE;
	blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendState;
}

D3D12_DEPTH_STENCIL_DESC CGBufferShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC depthStateRW = {};
	depthStateRW.DepthEnable = TRUE;
	depthStateRW.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStateRW.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStateRW.StencilEnable = FALSE;
	depthStateRW.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStateRW.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depthStateRW.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStateRW.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	return depthStateRW;
}

D3D12_INPUT_LAYOUT_DESC CGBufferShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	inputElementCount = 4;
	d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[2] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[3] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;
}

void CGBufferShader::CreatePipelineState(int stateNum)
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
	d3d12GraphicsPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3d12GraphicsPipelineState.NumRenderTargets = 3;
	d3d12GraphicsPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d12GraphicsPipelineState.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_SNORM;
	d3d12GraphicsPipelineState.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	d3d12GraphicsPipelineState.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	d3d12GraphicsPipelineState.SampleDesc.Count = 1;
	d3d12GraphicsPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	DX::ThrowIfFailed(d3d12Device->CreateGraphicsPipelineState(&d3d12GraphicsPipelineState, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(m_d3d12PipelineStates[stateNum].GetAddressOf())));

	if (d3d12GraphicsPipelineState.InputLayout.pInputElementDescs != nullptr)
	{
		delete[] d3d12GraphicsPipelineState.InputLayout.pInputElementDescs;
	}
}

D3D12_SHADER_BYTECODE CGBufferShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("GBuffer.hlsl", "VS_Main", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CGBufferShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("GBuffer.hlsl", "PS_Main", "ps_5_1", d3d12ShaderBlob);
}

ID3D12RootSignature* CGBufferShader::CreateRootSignature(int stateNum)
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	m_rootSignature = {};
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
	CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[1] = {};
	d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);
	CD3DX12_ROOT_PARAMETER d3d12RootParameters[1] = {};
	d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
	d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, 0, nullptr, rootSignatureFlags);
	ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
	if (d3d12ErrorBlob) {
		char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
		std::cerr << "VCT - Direct3D Error: " << pErrorString << std::endl;
	}
	DX::ThrowIfFailed(device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignature)));

	return m_rootSignature;
}