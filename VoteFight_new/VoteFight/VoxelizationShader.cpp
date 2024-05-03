#include "pch.h"
#include "Shader.h"
#include "Texture.h"
#include "GameFramework.h"
#include "VoxelizationShader.h"
#include "AssetManager.h"

CVoxelizationShader::CVoxelizationShader()
{
    // Voxelization 3D Render Target 咆胶媚 积己
    m_VCTVoxelization3DRT = new VCTTexture();

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    m_VCTVoxelization3DRT->SetName("Voxelization");
    m_VCTVoxelization3DRT->Create(static_cast<UINT64>(VCT_SCENE_VOLUME_SIZE), static_cast<UINT>(VCT_SCENE_VOLUME_SIZE), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, 0, VCT_SCENE_VOLUME_SIZE);

    // Texture狼 SRV 积己
    ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
    D3D12_CPU_DESCRIPTOR_HANDLE d3d12CpuDescriptorHandle = CGameFramework::GetInstance()->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE d3d12GpuDescriptorHandle = CGameFramework::GetInstance()->GetCbvSrvUavDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
    UINT descriptorIncrementSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    d3d12CpuDescriptorHandle.ptr += CAssetManager::GetInstance()->GetTextureCount() * descriptorIncrementSize;
    d3d12GpuDescriptorHandle.ptr += CAssetManager::GetInstance()->GetTextureCount() * descriptorIncrementSize;

    ID3D12Resource* pShaderResource = m_VCTVoxelization3DRT->GetTexture();
    D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
    d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    d3dShaderResourceViewDesc.Format = pShaderResource->GetDesc().Format;
    d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    d3dShaderResourceViewDesc.Texture3D.MipLevels = 1;

    d3d12Device->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, d3d12CpuDescriptorHandle);
    m_VCTVoxelization3DRT->SetGpuDescriptorHandle(d3d12GpuDescriptorHandle);
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE CVoxelizationShader::GetPrimitiveType(int stateNum)
{
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

ID3D12RootSignature* CVoxelizationShader::CreateRootSignature(int stateNum)
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12RootSignature* rootSignature = {};

    // Voxelization Root Signature 积己
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[3] = {};
    d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// Albedo
    d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	// Normal
    d3d12DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);	// Cube
    CD3DX12_ROOT_PARAMETER d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::COUNT)] = {};
    d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
    d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
    d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[2]);
    CD3DX12_STATIC_SAMPLER_DESC d3d12SamplerDesc[1] = {};
    d3d12SamplerDesc[0].Init(1, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL);
    CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
    d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, _countof(d3d12SamplerDesc), d3d12SamplerDesc, rootSignatureFlags);
    ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
    if (d3d12ErrorBlob) {
        char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
        std::cerr << "VCT - Direct3D Error: " << pErrorString << std::endl;
    }
    else {
        std::cerr << "VCT - Direct3D Error: Unknown error" << std::endl;
    }
    DX::ThrowIfFailed(device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(rootSignature)));

    return rootSignature;
}

CVoxelizationShader::~CVoxelizationShader()
{
}

D3D12_INPUT_LAYOUT_DESC CVoxelizationShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	inputElementCount = 1;
	d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;
}

D3D12_SHADER_BYTECODE CVoxelizationShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Voxeliaztion
		return Compile("VoxelConeTracingVoxelization.hlsl", "VS_Main", "vs_5_1", d3d12ShaderBlob);
	case 1: // VoxeliaztionDebug
	}

	return CShader::CreateVertexShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CVoxelizationShader::CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
    switch (stateNum)
    {
    case 0: // Voxeliaztion
        return Compile("VoxelConeTracingVoxelization.hlsl", "GS_Main", "gs_5_1", d3d12ShaderBlob);
    case 1: // VoxeliaztionDebug
    }

    return CShader::CreateGeometryShader(d3d12ShaderBlob, stateNum);
}

D3D12_SHADER_BYTECODE CVoxelizationShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	switch (stateNum)
	{
	case 0: // Voxeliaztion
		return Compile("VoxelConeTracingVoxelization.hlsl", "PS_Main", "ps_5_1", d3d12ShaderBlob);
	case 1: // VoxeliaztionDebug
	}

	return CShader::CreatePixelShader(d3d12ShaderBlob, stateNum);
}

D3D12_RASTERIZER_DESC CVoxelizationShader::CreateRasterizerState(int stateNum)
{
    D3D12_RASTERIZER_DESC rasterizerStateNoCullNoDepth = {};
    rasterizerStateNoCullNoDepth.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerStateNoCullNoDepth.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerStateNoCullNoDepth.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerStateNoCullNoDepth.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerStateNoCullNoDepth.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerStateNoCullNoDepth.DepthClipEnable = FALSE;
    rasterizerStateNoCullNoDepth.MultisampleEnable = FALSE;
    rasterizerStateNoCullNoDepth.AntialiasedLineEnable = FALSE;
    rasterizerStateNoCullNoDepth.ForcedSampleCount = 0;
    rasterizerStateNoCullNoDepth.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    return rasterizerStateNoCullNoDepth;
}

D3D12_BLEND_DESC CVoxelizationShader::CreateBlendState(int stateNum)
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

D3D12_DEPTH_STENCIL_DESC CVoxelizationShader::CreateDepthStencilState(int stateNum)
{
    D3D12_DEPTH_STENCIL_DESC depthStateRW = {};
    depthStateRW.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStateRW.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStateRW.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStateRW.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

    D3D12_DEPTH_STENCIL_DESC depthStateDisabled = {};
    depthStateDisabled.DepthEnable = FALSE;
    depthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStateDisabled.StencilEnable = FALSE;
    depthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthStateDisabled.BackFace = depthStateRW.FrontFace;

    return depthStateDisabled;
}

void CVoxelizationShader::Render()
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();

    DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_VCTVoxelization3DRT->UpdateShaderVariable();
    DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

}

void CVoxelizationShader::VCTTexture::Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, int rootSignature, int depth, int mips)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();

	rootSignature = rootSignature;
	m_d3d12Texture = DX::CreateTextureResource(d3d12Device, Width, Height, 1, 0, D3D12ResourceStates, D3D12ResourceFlags, DxgiFormat, D3D12ClearValue);
}

void CVoxelizationShader::VCTTexture::UpdateShaderVariable()
{
    ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

    d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(rootSignature, m_d3d12GpuDescriptorHandle);
}


