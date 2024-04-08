#include "pch.h"
#include "GameFramework.h"
#include "VoxelConeTracing.h"
#include "DescriptorHeap.h"
#include "Shader.h"
#include "Buffer.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"

DXRSRenderTarget::DXRSRenderTarget(DESC::DescriptorHeapManager* descriptorManager, int width, int height, DXGI_FORMAT aFormat, D3D12_RESOURCE_FLAGS flags, LPCWSTR name, int depth, int mips, D3D12_RESOURCE_STATES defaultState)
{
    ID3D12Device* device = CGameFramework::GetInstance()->GetDevice();

    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mFormat = aFormat;

    XMFLOAT4 clearColor = { 0, 0, 0, 1 };
    DXGI_FORMAT format = aFormat;

    // Describe and create a Texture2D/3D
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = mips;
    textureDesc.Format = format;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.DepthOrArraySize = (depth > 0) ? depth : 1;
    textureDesc.Flags = flags;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = (depth > 0) ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = format;
    optimizedClearValue.Color[0] = clearColor.x;
    optimizedClearValue.Color[1] = clearColor.y;
    optimizedClearValue.Color[2] = clearColor.z;
    optimizedClearValue.Color[3] = clearColor.w;

    mCurrentResourceState = defaultState;

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        mCurrentResourceState,
        &optimizedClearValue,
        IID_PPV_ARGS(&mRenderTarget)));

    mRenderTarget->SetName(name);

    D3D12_DESCRIPTOR_HEAP_FLAGS flagsHeap = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = aFormat;
    if (depth > 0) {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = mips;
    }
    else {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = mips;
    }

    mDescriptorSRV = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateShaderResourceView(mRenderTarget.Get(), &srvDesc, mDescriptorSRV.GetCPUHandle());

    mDescriptorUAVMipsHandles.resize(mips);
    mDescriptorRTVMipsHandles.resize(mips);

    for (int mipLevel = 0; mipLevel < mips; mipLevel++)
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = aFormat;
        if (depth > 0) {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = mipLevel;
            rtvDesc.Texture3D.WSize = (depth >> mipLevel);
        }
        else {
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = mipLevel;
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = aFormat;
        if (depth > 0) {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            uavDesc.Texture3D.MipSlice = mipLevel;
            uavDesc.Texture3D.WSize = (depth >> mipLevel);
        }
        else {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = mipLevel;
        }

        mDescriptorRTVMipsHandles[mipLevel] = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        device->CreateRenderTargetView(mRenderTarget.Get(), &rtvDesc, mDescriptorRTVMipsHandles[mipLevel].GetCPUHandle());

        if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
        {
            mDescriptorUAVMipsHandles[mipLevel] = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            device->CreateUnorderedAccessView(mRenderTarget.Get(), nullptr, &uavDesc, mDescriptorUAVMipsHandles[mipLevel].GetCPUHandle());
        }
    }

    //D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    //rtvDesc.Format = aFormat;
    //if (depth > 0) {
    //	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
    //	rtvDesc.Texture3D.MipSlice = 0;
    //	rtvDesc.Texture3D.WSize = depth;
    //}
    //else {
    //	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    //	rtvDesc.Texture2D.MipSlice = 0;
    //}

    //D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    //uavDesc.Format = aFormat;
    //if (depth > 0) {
    //	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
    //	uavDesc.Texture3D.MipSlice = 0;
    //	uavDesc.Texture3D.WSize = depth;
    //}
    //else {
    //	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    //	uavDesc.Texture2D.MipSlice = 0;
    //}

    //mDescriptorRTV = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //device->CreateRenderTargetView(mRenderTarget.Get(), &rtvDesc, mDescriptorRTV.GetCPUHandle());

    //if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
    //{
    //	mDescriptorUAV = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    //	device->CreateUnorderedAccessView(mRenderTarget.Get(), nullptr, &uavDesc, mDescriptorUAV.GetCPUHandle());
    //}
}

void DXRSRenderTarget::TransitionTo(std::vector<CD3DX12_RESOURCE_BARRIER>& barriers, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter)
{
    if (stateAfter != mCurrentResourceState)
    {
        barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(GetResource(), mCurrentResourceState, stateAfter));
        mCurrentResourceState = stateAfter;
    }
}

namespace {
    D3D12_HEAP_PROPERTIES UploadHeapProps = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };
    D3D12_HEAP_PROPERTIES DefaultHeapProps = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

    static const float clearColorBlack[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float clearColorWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void DXRSExampleGIScene::InitVoxelConeTracing(DESC::DescriptorHeapManager* descriptorManager)
{
    framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();

    // voxelization
    {
        D3D12_SAMPLER_DESC shadowSampler = {};
        shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        shadowSampler.MipLODBias = 0;
        shadowSampler.MaxAnisotropy = 16;
        shadowSampler.MinLOD = 0.0f;
        shadowSampler.MaxLOD = D3D12_FLOAT32_MAX;
        shadowSampler.BorderColor[0] = 1.0f;
        shadowSampler.BorderColor[1] = 1.0f;
        shadowSampler.BorderColor[2] = 1.0f;
        shadowSampler.BorderColor[3] = 1.0f;

        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        mVCTVoxelization3DRT = new DXRSRenderTarget(device, descriptorManager, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE, format, flags, L"Voxelization Scene Data 3D", VCT_SCENE_VOLUME_SIZE);
        mVCTVoxelization3DRT_CopyForAsync = new DXRSRenderTarget(device, descriptorManager, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE, format, flags, L"Voxelization Scene Data 3D Copy", VCT_SCENE_VOLUME_SIZE);

        //create root signature
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        mVCTVoxelizationRS.Reset(3, 1);
        mVCTVoxelizationRS.InitStaticSampler(0, shadowSampler, D3D12_SHADER_VISIBILITY_PIXEL);
        mVCTVoxelizationRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);
        mVCTVoxelizationRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTVoxelizationRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTVoxelizationRS.Finalize(device, L"VCT voxelization pass RS", rootSignatureFlags);

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> geometryShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ID3DBlob* errorBlob = nullptr;

        // CShader::Compile("Voxelization.hlsl", );
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelization.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vertexShader, &errorBlob));
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelization.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", compileFlags, 0, &geometryShader, &errorBlob));
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelization.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
           { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        mRasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        mRasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        mRasterizerState.FrontCounterClockwise = FALSE;
        mRasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        mRasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        mRasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        mRasterizerState.DepthClipEnable = TRUE;
        mRasterizerState.MultisampleEnable = FALSE;
        mRasterizerState.AntialiasedLineEnable = FALSE;
        mRasterizerState.ForcedSampleCount = 0;
        mRasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        mRasterizerStateNoCullNoDepth.FillMode = D3D12_FILL_MODE_SOLID;
        mRasterizerStateNoCullNoDepth.CullMode = D3D12_CULL_MODE_NONE;
        mRasterizerStateNoCullNoDepth.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        mRasterizerStateNoCullNoDepth.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        mRasterizerStateNoCullNoDepth.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        mRasterizerStateNoCullNoDepth.DepthClipEnable = FALSE;
        mRasterizerStateNoCullNoDepth.MultisampleEnable = FALSE;
        mRasterizerStateNoCullNoDepth.AntialiasedLineEnable = FALSE;
        mRasterizerStateNoCullNoDepth.ForcedSampleCount = 0;
        mRasterizerStateNoCullNoDepth.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        mBlendState.IndependentBlendEnable = FALSE;
        mBlendState.RenderTarget[0].BlendEnable = FALSE;
        mBlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        mBlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        mBlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        mBlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        mBlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        mBlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        mBlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        mDepthStateRW.DepthEnable = TRUE;
        mDepthStateRW.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        mDepthStateRW.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        mDepthStateRW.StencilEnable = FALSE;
        mDepthStateRW.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        mDepthStateRW.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        mDepthStateRW.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        mDepthStateRW.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateRW.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateRW.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateRW.BackFace = mDepthStateRW.FrontFace;

        mDepthStateDisabled.DepthEnable = FALSE;
        mDepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        mDepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        mDepthStateDisabled.StencilEnable = FALSE;
        mDepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        mDepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        mDepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        mDepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        mDepthStateDisabled.BackFace = mDepthStateRW.FrontFace;


        mVCTVoxelizationPSO.SetRootSignature(mVCTVoxelizationRS);
        mVCTVoxelizationPSO.SetRasterizerState(mRasterizerStateNoCullNoDepth);
        mVCTVoxelizationPSO.SetRenderTargetFormats(0, nullptr, DXGI_FORMAT_D32_FLOAT);
        mVCTVoxelizationPSO.SetBlendState(mBlendState);
        mVCTVoxelizationPSO.SetDepthStencilState(mDepthStateDisabled);
        mVCTVoxelizationPSO.SetInputLayout(_countof(inputElementDescs), inputElementDescs);
        mVCTVoxelizationPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        mVCTVoxelizationPSO.SetVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize());
        mVCTVoxelizationPSO.SetGeometryShader(geometryShader->GetBufferPointer(), geometryShader->GetBufferSize());
        mVCTVoxelizationPSO.SetPixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize());
        mVCTVoxelizationPSO.Finalize(device);

        //create constant buffer for pass
        CBuffer::Description cbDesc;
        cbDesc.mElementSize = sizeof(VCTVoxelizationCBData);
        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
        cbDesc.mDescriptorType = CBuffer::DescriptorType::CBV;
        mVCTVoxelizationCB = new CBuffer(device, descriptorManager, framework->GetGraphicsCommandList(), cbDesc, L"VCT Voxelization Pass CB");

        XMMATRIX mCameraView;
        XMMATRIX mCameraProjection;
        XMMATRIX mLightViewProjection;

        VCTVoxelizationCBData data = {};
        float scale = 1.0f;
        data.WorldVoxelCube = XMMatrixScaling(scale, scale, scale);
        data.ViewProjection = mCameraView * mCameraProjection;
        data.ShadowViewProjection = mLightViewProjection;
        data.WorldVoxelScale = mWorldVoxelScale;
        memcpy(mVCTVoxelizationCB->Map(), &data, sizeof(data));
    }

    //debug 
    {
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        mVCTVoxelizationDebugRT = new DXRSRenderTarget(device, descriptorManager, 1920, 1080, format, flags, L"Voxelization Debug RT");

        //create root signature
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        mVCTVoxelizationDebugRS.Reset(2, 0);
        mVCTVoxelizationDebugRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTVoxelizationDebugRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTVoxelizationDebugRS.Finalize(device, L"VCT voxelization debug pass RS", rootSignatureFlags);

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> geometryShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ID3DBlob* errorBlob = nullptr;

        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelizationDebug.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vertexShader, &errorBlob));
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelizationDebug.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", compileFlags, 0, &geometryShader, &errorBlob));
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelizationDebug.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));

        DXGI_FORMAT formats[1];
        formats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

        mVCTVoxelizationDebugPSO.SetRootSignature(mVCTVoxelizationDebugRS);
        mVCTVoxelizationDebugPSO.SetRasterizerState(mRasterizerState);
        mVCTVoxelizationDebugPSO.SetRenderTargetFormats(_countof(formats), formats, DXGI_FORMAT_D32_FLOAT);
        mVCTVoxelizationDebugPSO.SetBlendState(mBlendState);
        mVCTVoxelizationDebugPSO.SetDepthStencilState(mDepthStateRW);
        mVCTVoxelizationDebugPSO.SetInputLayout(0, nullptr);
        mVCTVoxelizationDebugPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
        mVCTVoxelizationDebugPSO.SetVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize());
        mVCTVoxelizationDebugPSO.SetGeometryShader(geometryShader->GetBufferPointer(), geometryShader->GetBufferSize());
        mVCTVoxelizationDebugPSO.SetPixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize());
        mVCTVoxelizationDebugPSO.Finalize(device);
    }

//    // aniso mipmapping prepare
//    {
//        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
//
//        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//
//        int size = VCT_SCENE_VOLUME_SIZE >> 1;
//        mVCTAnisoMipmappinPrepare3DRTs[0] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare X+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinPrepare3DRTs[1] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare X- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinPrepare3DRTs[2] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare Y+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinPrepare3DRTs[3] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare Y- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinPrepare3DRTs[4] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare Z+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinPrepare3DRTs[5] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Prepare Z- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//
//        mVCTAnisoMipmappingPrepareRS.Reset(3, 0);
//        mVCTAnisoMipmappingPrepareRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTAnisoMipmappingPrepareRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTAnisoMipmappingPrepareRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 6, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTAnisoMipmappingPrepareRS.Finalize(device, L"VCT aniso mipmapping prepare pass compute version RS", rootSignatureFlags);
//
//        ComPtr<ID3DBlob> computeShader;
//
//#if defined(_DEBUG)
//        // Enable better shader debugging with the graphics debugging tools.
//        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//        UINT compileFlags = 0;
//#endif
//        ID3DBlob* errorBlob = nullptr;
//
//        DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\VoxelConeTracingAnisoMipmapPrepareCS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
//        if (errorBlob)
//        {
//            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//            errorBlob->Release();
//        }
//
//        mVCTAnisoMipmappingPreparePSO.SetRootSignature(mVCTAnisoMipmappingPrepareRS);
//        mVCTAnisoMipmappingPreparePSO.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
//        mVCTAnisoMipmappingPreparePSO.Finalize(device);
//
//        DXRSBuffer::Description cbDesc;
//        cbDesc.mElementSize = sizeof(VCTAnisoMipmappingCBData);
//        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
//        cbDesc.mDescriptorType = DXRSBuffer::DescriptorType::CBV;
//
//        mVCTAnisoMipmappingCB = new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping CB");
//    }
//
//    // aniso mipmapping main
//    {
//        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
//
//        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//
//        int size = VCT_SCENE_VOLUME_SIZE >> 1;
//        mVCTAnisoMipmappinMain3DRTs[0] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main X+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinMain3DRTs[1] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main X- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinMain3DRTs[2] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main Y+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinMain3DRTs[3] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main Y- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinMain3DRTs[4] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main Z+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        mVCTAnisoMipmappinMain3DRTs[5] = new DXRSRenderTarget(device, descriptorManager, size, size, format, flags, L"Voxelization Scene Mip Main Z- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//
//        mVCTAnisoMipmappingMainRS.Reset(2, 0);
//        mVCTAnisoMipmappingMainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTAnisoMipmappingMainRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 12, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTAnisoMipmappingMainRS.Finalize(device, L"VCT aniso mipmapping main pass comptue version RS", rootSignatureFlags);
//
//        ComPtr<ID3DBlob> computeShader;
//
//#if defined(_DEBUG)
//        // Enable better shader debugging with the graphics debugging tools.
//        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//        UINT compileFlags = 0;
//#endif
//        ID3DBlob* errorBlob = nullptr;
//
//        DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\VoxelConeTracingAnisoMipmapMainCS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
//        if (errorBlob)
//        {
//            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//            errorBlob->Release();
//        }
//
//        mVCTAnisoMipmappingMainPSO.SetRootSignature(mVCTAnisoMipmappingMainRS);
//        mVCTAnisoMipmappingMainPSO.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
//        mVCTAnisoMipmappingMainPSO.Finalize(device);
//
//        DXRSBuffer::Description cbDesc;
//        cbDesc.mElementSize = sizeof(VCTAnisoMipmappingCBData);
//        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
//        cbDesc.mDescriptorType = DXRSBuffer::DescriptorType::CBV;
//
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 0 CB"));
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 1 CB"));
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 2 CB"));
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 3 CB"));
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 4 CB"));
//        mVCTAnisoMipmappingMainCB.push_back(new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT aniso mip mapping main mip 5 CB"));
//
//    }
//
//    // main 
//    {
//        DXRSBuffer::Description cbDesc;
//        cbDesc.mElementSize = sizeof(VCTMainCBData);
//        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
//        cbDesc.mDescriptorType = DXRSBuffer::DescriptorType::CBV;
//
//        mVCTMainCB = new DXRSBuffer(framework->GetD3DDevice(), descriptorManager, framework->GetCommandListGraphics(), cbDesc, L"VCT main CB");
//
//        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
//
//        mVCTMainRT = new DXRSRenderTarget(device, descriptorManager, MAX_SCREEN_WIDTH * mVCTRTRatio, MAX_SCREEN_HEIGHT * mVCTRTRatio, format, flags, L"VCT Final Output", -1, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//
//        //create root signature
//        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
//
//        // PS
//        {
//            mVCTMainRS.Reset(2, 1);
//            mVCTMainRS.InitStaticSampler(0, mBilinearSamplerClamp, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS.Finalize(device, L"VCT main pass pixel version RS", rootSignatureFlags);
//
//            ComPtr<ID3DBlob> vertexShader;
//            ComPtr<ID3DBlob> pixelShader;
//
//#if defined(_DEBUG)
//            // Enable better shader debugging with the graphics debugging tools.
//            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//            UINT compileFlags = 0;
//#endif
//
//            ID3DBlob* errorBlob = nullptr;
//
//            DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\VoxelConeTracingPS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vertexShader, &errorBlob));
//            if (errorBlob)
//            {
//                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//                errorBlob->Release();
//            }
//            DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\VoxelConeTracingPS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));
//            if (errorBlob)
//            {
//                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//                errorBlob->Release();
//            }
//            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
//            {
//               { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
//               { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
//            };
//
//            DXGI_FORMAT formats[1];
//            formats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//            mVCTMainPSO.SetRootSignature(mVCTMainRS);
//            mVCTMainPSO.SetRasterizerState(mRasterizerState);
//            mVCTMainPSO.SetBlendState(mBlendState);
//            mVCTMainPSO.SetDepthStencilState(mDepthStateDisabled);
//            mVCTMainPSO.SetInputLayout(_countof(inputElementDescs), inputElementDescs);
//            mVCTMainPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
//            mVCTMainPSO.SetRenderTargetFormats(_countof(formats), formats, DXGI_FORMAT_D32_FLOAT);
//            mVCTMainPSO.SetVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize());
//            mVCTMainPSO.SetPixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize());
//            mVCTMainPSO.Finalize(device);
//        }
//
//        // CS
//        {
//            mVCTMainRS_Compute.Reset(3, 1);
//            mVCTMainRS_Compute.InitStaticSampler(0, mBilinearSamplerClamp, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS_Compute[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS_Compute[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS_Compute[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//            mVCTMainRS_Compute.Finalize(device, L"VCT main pass compute version RS", rootSignatureFlags);
//
//            ComPtr<ID3DBlob> computeShader;
//
//#if defined(_DEBUG)
//            // Enable better shader debugging with the graphics debugging tools.
//            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//            UINT compileFlags = 0;
//#endif
//            ID3DBlob* errorBlob = nullptr;
//
//            DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\VoxelConeTracingCS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
//            if (errorBlob)
//            {
//                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//                errorBlob->Release();
//            }
//
//            mVCTMainPSO_Compute.SetRootSignature(mVCTMainRS_Compute);
//            mVCTMainPSO_Compute.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
//            mVCTMainPSO_Compute.Finalize(device);
//        }
//    }
//
//    // upsample and blur
//    {
//        //RTs
//        mVCTMainUpsampleAndBlurRT = new DXRSRenderTarget(device, descriptorManager, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT,
//            DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, L"VCT Main RT Upsampled & Blurred", -1, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//
//        //create root signature
//        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
//            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
//            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
//
//        mVCTMainUpsampleAndBlurRS.Reset(3, 1);
//        mVCTMainUpsampleAndBlurRS.InitStaticSampler(0, mBilinearSamplerClamp, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTMainUpsampleAndBlurRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTMainUpsampleAndBlurRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTMainUpsampleAndBlurRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
//        mVCTMainUpsampleAndBlurRS.Finalize(device, L"VCT Main RT Upsample & Blur pass RS", rootSignatureFlags);
//
//        ComPtr<ID3DBlob> computeShader;
//
//#if defined(_DEBUG)
//        // Enable better shader debugging with the graphics debugging tools.
//        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//        UINT compileFlags = 0;
//#endif
//        ID3DBlob* errorBlob = nullptr;
//
//        DX::ThrowIfFailed(D3DCompileFromFile(framework->GetFilePath(L"content\\shaders\\UpsampleBlurCS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
//        if (errorBlob)
//        {
//            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
//            errorBlob->Release();
//        }
//
//        mVCTMainUpsampleAndBlurPSO.SetRootSignature(mVCTMainUpsampleAndBlurRS);
//        mVCTMainUpsampleAndBlurPSO.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
//        mVCTMainUpsampleAndBlurPSO.Finalize(device);
//    }
}

void DXRSExampleGIScene::RenderObject(std::unique_ptr<CObject>& aModel, std::function<void(std::unique_ptr<CObject>&)> aCallback)
{
    if (aCallback) {
        aCallback(aModel);
    }
}


void DXRSExampleGIScene::RenderVoxelConeTracing(DESC::GPUDescriptorHeap* gpuDescriptorHeap, RenderQueue aQueue, bool useAsyncCompute)
{
    framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();

    RECT rect;
    GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &rect);

    CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, rect.right, rect.bottom);
    CD3DX12_RECT rect = CD3DX12_RECT(0.0f, 0.0f, rect.right, rect.bottom);

    auto clearVCTMainRT = [this, commandList]() {
        commandList->SetPipelineState(mVCTMainPSO.GetPipelineStateObject());
        commandList->SetGraphicsRootSignature(mVCTMainRS.GetSignature());

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesRSM[] = { mVCTMainRT->GetRTV().GetCPUHandle() };
        D3D12_CPU_DESCRIPTOR_HANDLE uavHandlesRSM[] = { mVCTMainUpsampleAndBlurRT->GetUAV().GetCPUHandle() };

        //transition buffers to rendertarget outputs
        framework->ResourceBarriersBegin(mBarriers);
        mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        framework->ResourceBarriersEnd(mBarriers, commandList);

        commandList->OMSetRenderTargets(_countof(rtvHandlesRSM), rtvHandlesRSM, FALSE, nullptr);
        commandList->ClearRenderTargetView(rtvHandlesRSM[0], clearColorBlack, 0, nullptr);
        };

    CD3DX12_VIEWPORT vctViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE);
    CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE);
    commandList->RSSetViewports(1, &vctViewport);
    commandList->RSSetScissorRects(1, &vctRect);
    commandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);

    commandList->SetPipelineState(mVCTVoxelizationPSO.GetPipelineStateObject());
    commandList->SetGraphicsRootSignature(mVCTVoxelizationRS.GetSignature());

    framework->ResourceBarriersBegin(mBarriers);
    mVCTVoxelization3DRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    framework->ResourceBarriersEnd(mBarriers, commandList);

    DESC::DescriptorHandle cbvHandle;
    DESC::DescriptorHandle uavHandle;
    DESC::DescriptorHandle srvHandle;
    uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
    gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTVoxelization3DRT->GetUAV());

    commandList->ClearUnorderedAccessViewFloat(uavHandle.GetGPUHandle(), mVCTVoxelization3DRT->GetUAV().GetCPUHandle(), mVCTVoxelization3DRT->GetResource(), clearColorBlack, 0, nullptr);

    srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
    gpuDescriptorHeap->AddToHandle(device, srvHandle, mShadowDepth->GetSRV());

    for (auto& model : mRenderableObjects) {
        RenderObject(model, [this, gpuDescriptorHeap, commandList, &cbvHandle, &uavHandle, &srvHandle, device](std::unique_ptr<CObject>& anObject) {
            cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());
            gpuDescriptorHeap->AddToHandle(device, cbvHandle, anObject->GetCB()->GetCBV());

            commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
            commandList->SetGraphicsRootDescriptorTable(1, srvHandle.GetGPUHandle());
            commandList->SetGraphicsRootDescriptorTable(2, uavHandle.GetGPUHandle());

            anObject->Render();
            });
    }

    //reset back
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &rect);


    commandList->SetPipelineState(mVCTVoxelizationDebugPSO.GetPipelineStateObject());
    commandList->SetGraphicsRootSignature(mVCTVoxelizationDebugRS.GetSignature());

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesFinal[] =
    {
        mVCTVoxelizationDebugRT->GetRTV().GetCPUHandle()
    };

    commandList->OMSetRenderTargets(_countof(rtvHandlesFinal), rtvHandlesFinal, FALSE, &framework->GetDepthStencilView());
    commandList->ClearRenderTargetView(rtvHandlesFinal[0], clearColorBlack, 0, nullptr);
    commandList->ClearDepthStencilView(framework->GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    framework->ResourceBarriersBegin(mBarriers);
    mVCTVoxelization3DRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    framework->ResourceBarriersEnd(mBarriers, commandList);

    DESC::DescriptorHandle cbvHandle;
    DESC::DescriptorHandle uavHandle;

    cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
    gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());

    uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
    gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTVoxelization3DRT->GetUAV());

    commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
    commandList->SetGraphicsRootDescriptorTable(1, uavHandle.GetGPUHandle());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    commandList->DrawInstanced(VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE, 1, 0, 0);

    //if (!useAsyncCompute || (useAsyncCompute && aQueue == COMPUTE_QUEUE)) {
    //    PIXBeginEvent(commandList, 0, "VCT Mipmapping prepare CS");
    //    {
    //        commandList->SetPipelineState(mVCTAnisoMipmappingPreparePSO.GetPipelineStateObject());
    //        commandList->SetComputeRootSignature(mVCTAnisoMipmappingPrepareRS.GetSignature());

    //        framework->ResourceBarriersBegin(mBarriers);
    //        mVCTAnisoMipmappinPrepare3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        framework->ResourceBarriersEnd(mBarriers, commandList);

    //        VCTAnisoMipmappingCBData cbData = {};
    //        cbData.MipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
    //        cbData.MipLevel = 0;
    //        memcpy(mVCTAnisoMipmappingCB->Map(), &cbData, sizeof(cbData));
    //        DXRS::DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
    //        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTAnisoMipmappingCB->GetCBV());

    //        DXRS::DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
    //        gpuDescriptorHeap->AddToHandle(device, srvHandle, useAsyncCompute ? mVCTVoxelization3DRT_CopyForAsync->GetSRV() : mVCTVoxelization3DRT->GetSRV());

    //        DXRS::DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(6);
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[0]->GetUAV());
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[1]->GetUAV());
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[2]->GetUAV());
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[3]->GetUAV());
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[4]->GetUAV());
    //        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[5]->GetUAV());

    //        commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
    //        commandList->SetComputeRootDescriptorTable(1, srvHandle.GetGPUHandle());
    //        commandList->SetComputeRootDescriptorTable(2, uavHandle.GetGPUHandle());

    //        commandList->Dispatch(DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u));
    //    }
    //    PIXEndEvent(commandList);

    //    PIXBeginEvent(commandList, 0, "VCT Mipmapping main CS");
    //    {
    //        commandList->SetPipelineState(mVCTAnisoMipmappingMainPSO.GetPipelineStateObject());
    //        commandList->SetComputeRootSignature(mVCTAnisoMipmappingMainRS.GetSignature());

    //        framework->ResourceBarriersBegin(mBarriers);
    //        mVCTAnisoMipmappinPrepare3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinPrepare3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //        mVCTAnisoMipmappinMain3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinMain3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinMain3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinMain3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinMain3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //        mVCTAnisoMipmappinMain3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    //        framework->ResourceBarriersEnd(mBarriers, commandList);

    //        int mipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
    //        for (int mip = 0; mip < VCT_MIPS; mip++)
    //        {
    //            DXRS::DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTAnisoMipmappingMainCB[mip]->GetCBV());

    //            DXRS::DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(12);
    //            if (mip == 0) {
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[0]->GetUAV());
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[1]->GetUAV());
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[2]->GetUAV());
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[3]->GetUAV());
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[4]->GetUAV());
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[5]->GetUAV());
    //            }
    //            else {
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetUAV(mip - 1));
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetUAV(mip - 1));
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetUAV(mip - 1));
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetUAV(mip - 1));
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetUAV(mip - 1));
    //                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetUAV(mip - 1));
    //            }
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetUAV(mip));
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetUAV(mip));
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetUAV(mip));
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetUAV(mip));
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetUAV(mip));
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetUAV(mip));

    //            VCTAnisoMipmappingCBData cbData = {};
    //            cbData.MipDimension = mipDimension;
    //            cbData.MipLevel = mip;
    //            memcpy(mVCTAnisoMipmappingMainCB[mip]->Map(), &cbData, sizeof(cbData));
    //            commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
    //            commandList->SetComputeRootDescriptorTable(1, uavHandle.GetGPUHandle());

    //            commandList->Dispatch(DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u));
    //            mipDimension >>= 1;
    //        }
    //    }
    //    PIXEndEvent(commandList);

    //    if (!mVCTUseMainCompute && !useAsyncCompute) {
    //        PIXBeginEvent(commandList, 0, "VCT Main PS");
    //        {
    //            CD3DX12_VIEWPORT vctResViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, mVCTMainRT->GetWidth(), mVCTMainRT->GetHeight());
    //            CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, mVCTMainRT->GetWidth(), mVCTMainRT->GetHeight());
    //            commandList->RSSetViewports(1, &vctResViewport);
    //            commandList->RSSetScissorRects(1, &vctRect);

    //            commandList->SetPipelineState(mVCTMainPSO.GetPipelineStateObject());
    //            commandList->SetGraphicsRootSignature(mVCTMainRS.GetSignature());

    //            framework->ResourceBarriersBegin(mBarriers);
    //            mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    //            framework->ResourceBarriersEnd(mBarriers, commandList);

    //            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesFinal[] =
    //            {
    //                mVCTMainRT->GetRTV().GetCPUHandle()
    //            };

    //            commandList->OMSetRenderTargets(_countof(rtvHandlesFinal), rtvHandlesFinal, FALSE, nullptr);
    //            commandList->ClearRenderTargetView(rtvHandlesFinal[0], clearColorBlack, 0, nullptr);

    //            DXRS::DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(10);
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[0]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[1]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[2]->GetSRV());

    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT->GetSRV());

    //            DXRS::DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTMainCB->GetCBV());

    //            commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
    //            commandList->SetGraphicsRootDescriptorTable(1, srvHandle.GetGPUHandle());

    //            commandList->IASetVertexBuffers(0, 1, &framework->GetFullscreenQuadBufferView());
    //            commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    //            commandList->DrawInstanced(4, 1, 0, 0);

    //            //reset back
    //            commandList->RSSetViewports(1, &viewport);
    //            commandList->RSSetScissorRects(1, &rect);
    //        }
    //        PIXEndEvent(commandList);
    //    }
    //    else {
    //        PIXBeginEvent(commandList, 0, "VCT Main CS");
    //        {
    //            commandList->SetPipelineState(mVCTMainPSO_Compute.GetPipelineStateObject());
    //            commandList->SetComputeRootSignature(mVCTMainRS_Compute.GetSignature());

    //            framework->ResourceBarriersBegin(mBarriers);
    //            mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //            framework->ResourceBarriersEnd(mBarriers, commandList);

    //            DXRS::DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
    //            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTMainRT->GetUAV());

    //            DXRS::DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(10);
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[0]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[1]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[2]->GetSRV());

    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetSRV());
    //            gpuDescriptorHeap->AddToHandle(device, srvHandle, useAsyncCompute ? mVCTVoxelization3DRT_CopyForAsync->GetSRV() : mVCTVoxelization3DRT->GetSRV());

    //            DXRS::DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTMainCB->GetCBV());

    //            commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
    //            commandList->SetComputeRootDescriptorTable(1, srvHandle.GetGPUHandle());
    //            commandList->SetComputeRootDescriptorTable(2, uavHandle.GetGPUHandle());

    //            commandList->Dispatch(DivideByMultiple(static_cast<UINT>(mVCTMainRT->GetWidth()), 8u), DivideByMultiple(static_cast<UINT>(mVCTMainRT->GetHeight()), 8u), 1u);
    //        }
    //        PIXEndEvent(commandList);
    //    }

    //    // upsample and blur
    //    if (mVCTMainRTUseUpsampleAndBlur) {
    //        PIXBeginEvent(commandList, 0, "VCT Main RT upsample & blur CS");
    //        {
    //            commandList->SetPipelineState(mVCTMainUpsampleAndBlurPSO.GetPipelineStateObject());
    //            commandList->SetComputeRootSignature(mVCTMainUpsampleAndBlurRS.GetSignature());

    //            framework->ResourceBarriersBegin(mBarriers);
    //            mVCTMainUpsampleAndBlurRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //            framework->ResourceBarriersEnd(mBarriers, commandList);

    //            DXRS::DescriptorHandle srvHandleBlur = gpuDescriptorHeap->GetHandleBlock(1);
    //            gpuDescriptorHeap->AddToHandle(device, srvHandleBlur, mVCTMainRT->GetSRV());

    //            DXRS::DescriptorHandle uavHandleBlur = gpuDescriptorHeap->GetHandleBlock(1);
    //            gpuDescriptorHeap->AddToHandle(device, uavHandleBlur, mVCTMainUpsampleAndBlurRT->GetUAV());

    //            DXRS::DescriptorHandle cbvHandleBlur = gpuDescriptorHeap->GetHandleBlock(1);
    //            gpuDescriptorHeap->AddToHandle(device, cbvHandleBlur, mGIUpsampleAndBlurBuffer->GetCBV());

    //            commandList->SetComputeRootDescriptorTable(0, srvHandleBlur.GetGPUHandle());
    //            commandList->SetComputeRootDescriptorTable(1, uavHandleBlur.GetGPUHandle());
    //            commandList->SetComputeRootDescriptorTable(2, cbvHandleBlur.GetGPUHandle());

    //            commandList->Dispatch(DivideByMultiple(static_cast<UINT>(mVCTMainUpsampleAndBlurRT->GetWidth()), 8u), DivideByMultiple(static_cast<UINT>(mVCTMainUpsampleAndBlurRT->GetHeight()), 8u), 1u);
    //        }
    //        PIXEndEvent(commandList);
    //    }
    //}

}