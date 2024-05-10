#include "pch.h"
#include "Shader.h"

#include "GameFramework.h"

#include "AssetManager.h"

CShader::CShader() :
	m_d3d12PipelineStates()
{
}

CShader::~CShader()
{
}

D3D12_SHADER_BYTECODE CShader::Compile(const string& fileName, const string& shaderName, const string& shaderVersion, ID3DBlob* d3d12CodeBlob)
{
	D3D12_SHADER_BYTECODE d3d12ShaderBytecode = {};
	UINT compileFlags = 0;

#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Shader\\" + fileName;
	ComPtr<ID3DBlob> d3d12ErrorBlob = nullptr;

	DX::ThrowIfFailed(D3DCompileFromFile(Utility::ConvertString(filePath).c_str(), nullptr, nullptr, shaderName.c_str(), shaderVersion.c_str(), compileFlags, 0, &d3d12CodeBlob, d3d12ErrorBlob.GetAddressOf()));

	if (d3d12CodeBlob != nullptr)
	{
		d3d12ShaderBytecode.BytecodeLength = d3d12CodeBlob->GetBufferSize();
		d3d12ShaderBytecode.pShaderBytecode = d3d12CodeBlob->GetBufferPointer();
	}
	else if (d3d12ErrorBlob.Get() != nullptr)
	{
		char* errorMessage = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
		size_t errorMessageSize = d3d12ErrorBlob->GetBufferSize();

		// 문자열 출력 또는 다른 처리
		std::string errorString(errorMessage, errorMessageSize);
		OutputDebugStringA(errorString.c_str());

		OutputDebugString(TEXT("쉐이더를 컴파일하지 못했습니다.\n"));
	}

	return d3d12ShaderBytecode;
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout(int stateNum)
{
	D3D12_INPUT_LAYOUT_DESC d3d12InputLayoutDesc = {};

	return d3d12InputLayoutDesc;
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState(int stateNum)
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

D3D12_BLEND_DESC CShader::CreateBlendState(int stateNum)
{
	D3D12_BLEND_DESC d3d12BlendDesc = {};

	d3d12BlendDesc.AlphaToCoverageEnable = false;
	d3d12BlendDesc.IndependentBlendEnable = false;
	d3d12BlendDesc.RenderTarget[0].BlendEnable = false;
	d3d12BlendDesc.RenderTarget[0].LogicOpEnable = false;
	d3d12BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3d12BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	d3d12BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3d12BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3d12BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3d12BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3d12BlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3d12BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return d3d12BlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC d3d12DepthStencilDesc = {};

	d3d12DepthStencilDesc.DepthEnable = true;
	d3d12DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3d12DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3d12DepthStencilDesc.StencilEnable = false;
	d3d12DepthStencilDesc.StencilReadMask = 0xff;
	d3d12DepthStencilDesc.StencilWriteMask = 0xff;
	d3d12DepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3d12DepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3d12DepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return d3d12DepthStencilDesc;
}

D3D12_STREAM_OUTPUT_DESC CShader::CreateStreamOutputState(int stateNum)
{
	D3D12_STREAM_OUTPUT_DESC d3d12StreamOutputDesc = {};

	return d3d12StreamOutputDesc;
}

ID3D12RootSignature* CShader::CreateRootSignature(int stateNum)
{
	return CGameFramework::GetInstance()->GetRootSignature();
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE d3d12ShaderByteCode = {};

	return d3d12ShaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreateHullShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE d3d12ShaderByteCode = {};

	return d3d12ShaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreateDomainShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE d3d12ShaderByteCode = {};

	return d3d12ShaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE d3d12ShaderByteCode = {};

	return d3d12ShaderByteCode;
}

D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE d3d12ShaderByteCode = {};

	return d3d12ShaderByteCode;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE CShader::GetPrimitiveType(int stateNum)
{
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

DXGI_FORMAT CShader::GetRTVFormat(int stateNum)
{
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

DXGI_FORMAT CShader::GetDSVFormat(int stateNum)
{
	return DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void CShader::CreatePipelineState(int stateNum)
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
	d3d12GraphicsPipelineState.NumRenderTargets = 1;
	d3d12GraphicsPipelineState.RTVFormats[0] = GetRTVFormat(stateNum);
	d3d12GraphicsPipelineState.DSVFormat = GetDSVFormat(stateNum);
	d3d12GraphicsPipelineState.SampleDesc.Count = 1;
	d3d12GraphicsPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	DX::ThrowIfFailed(d3d12Device->CreateGraphicsPipelineState(&d3d12GraphicsPipelineState, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(m_d3d12PipelineStates[stateNum].GetAddressOf())));

	if (d3d12GraphicsPipelineState.InputLayout.pInputElementDescs != nullptr)
	{
		delete[] d3d12GraphicsPipelineState.InputLayout.pInputElementDescs;
	}
}

void CShader::CreatePipelineStates(int stateCount)
{
	m_d3d12PipelineStates.resize(stateCount);

	for (int i = 0; i < stateCount; ++i)
	{
		CreatePipelineState(i);
	}
}

void CShader::SetPipelineState(int stateNum)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetPipelineState(m_d3d12PipelineStates[stateNum].Get());
}

//=========================================================================================================================

CComputeShader::CComputeShader() :
	m_threadGroup()
{
}

D3D12_SHADER_BYTECODE CComputeShader::CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	D3D12_SHADER_BYTECODE D3D12ShaderByteCode = {};

	return D3D12ShaderByteCode;
}

CComputeShader::~CComputeShader()
{
}

void CComputeShader::SetThreadGroup(const XMUINT3& threadGroup)
{
	m_threadGroup = threadGroup;
}

const XMUINT3& CComputeShader::GetThreadGroup()
{
	return m_threadGroup;
}

void CComputeShader::CreatePipelineState(int stateNum)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12RootSignature* d3d12RootSignature = CGameFramework::GetInstance()->GetRootSignature();
	D3D12_CACHED_PIPELINE_STATE D3D12CachedPipelineState = {};
	D3D12_COMPUTE_PIPELINE_STATE_DESC D3D12ComputePipelineStateDesc = {};
	ComPtr<ID3DBlob> D3D12ComputeShaderBlob = {};

	D3D12ComputePipelineStateDesc.pRootSignature = CreateRootSignature(stateNum);
	D3D12ComputePipelineStateDesc.CS = CreateComputeShader(D3D12ComputeShaderBlob.Get(), stateNum);
	D3D12ComputePipelineStateDesc.NodeMask = 0;
	D3D12ComputePipelineStateDesc.CachedPSO = D3D12CachedPipelineState;
	D3D12ComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	DX::ThrowIfFailed(d3d12Device->CreateComputePipelineState(&D3D12ComputePipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(m_d3d12PipelineStates[stateNum].GetAddressOf())));
}

void CComputeShader::Dispatch(int stateNum)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->SetPipelineState(m_d3d12PipelineStates[stateNum].Get());
	d3d12GraphicsCommandList->Dispatch(m_threadGroup.x, m_threadGroup.y, m_threadGroup.z);
}
