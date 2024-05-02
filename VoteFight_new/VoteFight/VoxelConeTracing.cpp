#include "pch.h"
#include "Mesh.h"
#include "DescriptorHeap.h"
#include "GameFramework.h"
#include "Shader.h"
#include "Buffer.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "VoxelConeTracing.h"
#include "SceneManager.h"
#include "Scene.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"
#include "Texture.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "Camera.h"

namespace {
    D3D12_HEAP_PROPERTIES UploadHeapProps = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };
    D3D12_HEAP_PROPERTIES DefaultHeapProps = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

    static const float clearColorBlack[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const float clearColorWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void VCT::InitVoxelConeTracing()
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    DescriptorHeapManager* descriptorManager = framework->GetDescriptorHeapManager();

    DXGI_FORMAT rtFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    mGbufferRTs[0] = new CRenderTarget(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, rtFormat, flags, L"Albedo");
    rtFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
    mGbufferRTs[1] = new CRenderTarget(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, rtFormat, flags, L"Normals");
    rtFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    mGbufferRTs[2] = new CRenderTarget(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, rtFormat, flags, L"World Positions");


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

        mVCTVoxelization3DRT = new CRenderTarget(VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE, format, flags, L"Voxelization Scene Data 3D", VCT_SCENE_VOLUME_SIZE);
        mVCTVoxelization3DRT_CopyForAsync = new CRenderTarget(VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE, format, flags, L"Voxelization Scene Data 3D Copy", VCT_SCENE_VOLUME_SIZE);

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
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelization.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", compileFlags, 0, &geometryShader, &errorBlob));
        if (errorBlob)
        {
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelization.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));
        if (errorBlob)
        {
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }

        // Define the vertex input layout.
        //D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        //{
        //   { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //   { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //   { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //   { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        //};

        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
           { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
           { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

        mBilinearSamplerClamp.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        mBilinearSamplerClamp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        mBilinearSamplerClamp.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        mBilinearSamplerClamp.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        //bilinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_EQUAL;
        mBilinearSamplerClamp.MipLODBias = 0;
        mBilinearSamplerClamp.MaxAnisotropy = 16;
        mBilinearSamplerClamp.MinLOD = 0.0f;
        mBilinearSamplerClamp.MaxLOD = D3D12_FLOAT32_MAX;
        mBilinearSamplerClamp.BorderColor[0] = 0.0f;
        mBilinearSamplerClamp.BorderColor[1] = 0.0f;
        mBilinearSamplerClamp.BorderColor[2] = 0.0f;
        mBilinearSamplerClamp.BorderColor[3] = 0.0f;


        //create constant buffer for pass
        CBuffer::Description cbDesc;
        cbDesc.mElementSize = sizeof(VCTVoxelizationCBData);
        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
        cbDesc.mDescriptorType = CBuffer::DescriptorType::CBV;
        mVCTVoxelizationCB = new CBuffer(cbDesc, L"VCT Voxelization Pass CB");

        CCamera* pCamera = CCameraManager::GetInstance()->GetMainCamera();
        XMMATRIX mCameraView = XMLoadFloat4x4(&pCamera->GetViewMatrix());
        XMMATRIX mCameraProjection = XMLoadFloat4x4(&pCamera->GetProjectionMatrix());
        XMMATRIX mLightViewProjection = XMMatrixIdentity();

        VCTVoxelizationCBData data = {};
        float scale = 1.0f;
        data.WorldVoxelCube = XMMatrixScaling(scale, scale, scale);
        data.ViewProjection = mCameraView * mCameraProjection;
        data.ShadowViewProjection = mLightViewProjection;
        data.WorldVoxelScale = mWorldVoxelScale;
        memcpy(mVCTVoxelizationCB->Map(), &data, sizeof(data));
    }

    {
        // Debug
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        mVCTVoxelizationDebugRT = new CRenderTarget(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, format, flags, L"Voxelization Debug RT");

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
        if (errorBlob)
        {
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelizationDebug.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", compileFlags, 0, &geometryShader, &errorBlob));
        if (errorBlob)
        {
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }
        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingVoxelizationDebug.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));
        if (errorBlob)
        {
            cout << (char*)errorBlob->GetBufferPointer() << endl;
            errorBlob->Release();
        }

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

        CObject* object = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::STRUCTURE)[0];
        mRenderableObjects.emplace_back(object);
    }

    // aniso mipmapping prepare
    {
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        int size = VCT_SCENE_VOLUME_SIZE >> 1;
        mVCTAnisoMipmappinPrepare3DRTs[0] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare X+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[1] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare X- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[2] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare Y+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[3] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare Y- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[4] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare Z+ 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[5] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Prepare Z- 3D", size, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        mVCTAnisoMipmappingPrepareRS.Reset(3, 0);
        mVCTAnisoMipmappingPrepareRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTAnisoMipmappingPrepareRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTAnisoMipmappingPrepareRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 6, D3D12_SHADER_VISIBILITY_ALL);
        mVCTAnisoMipmappingPrepareRS.Finalize(device, L"VCT aniso mipmapping prepare pass compute version RS", rootSignatureFlags);

        ComPtr<ID3DBlob> computeShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        ID3DBlob* errorBlob = nullptr;

        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingAnisoMipmapPrepareCS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        mVCTAnisoMipmappingPreparePSO.SetRootSignature(mVCTAnisoMipmappingPrepareRS);
        mVCTAnisoMipmappingPreparePSO.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
        mVCTAnisoMipmappingPreparePSO.Finalize(device);

        CBuffer::Description cbDesc;
        cbDesc.mElementSize = sizeof(VCTAnisoMipmappingCBData);
        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
        cbDesc.mDescriptorType = CBuffer::DescriptorType::CBV;

        mVCTAnisoMipmappingCB = new CBuffer(cbDesc, L"VCT aniso mip mapping CB");
    }

    // aniso mipmapping main
    {
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        int size = VCT_SCENE_VOLUME_SIZE >> 1;
        mVCTAnisoMipmappinMain3DRTs[0] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main X+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[1] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main X- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[2] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main Y+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[3] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main Y- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[4] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main Z+ 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[5] = new CRenderTarget(size, size, format, flags, L"Voxelization Scene Mip Main Z- 3D", size, VCT_MIPS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        mVCTAnisoMipmappingMainRS.Reset(2, 0);
        mVCTAnisoMipmappingMainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
        mVCTAnisoMipmappingMainRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 12, D3D12_SHADER_VISIBILITY_ALL);
        mVCTAnisoMipmappingMainRS.Finalize(device, L"VCT aniso mipmapping main pass comptue version RS", rootSignatureFlags);

        ComPtr<ID3DBlob> computeShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        ID3DBlob* errorBlob = nullptr;

        DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingAnisoMipmapMainCS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
        if (errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        mVCTAnisoMipmappingMainPSO.SetRootSignature(mVCTAnisoMipmappingMainRS);
        mVCTAnisoMipmappingMainPSO.SetComputeShader(computeShader->GetBufferPointer(), computeShader->GetBufferSize());
        mVCTAnisoMipmappingMainPSO.Finalize(device);

        CBuffer::Description cbDesc;
        cbDesc.mElementSize = sizeof(VCTAnisoMipmappingCBData);
        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
        cbDesc.mDescriptorType = CBuffer::DescriptorType::CBV;

        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 0 CB"));
        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 1 CB"));
        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 2 CB"));
        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 3 CB"));
        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 4 CB"));
        mVCTAnisoMipmappingMainCB.push_back(new CBuffer(cbDesc, L"VCT aniso mip mapping main mip 5 CB"));

    }

    // main 
    {
        CBuffer::Description cbDesc;
        cbDesc.mElementSize = sizeof(VCTMainCBData);
        cbDesc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
        cbDesc.mDescriptorType = CBuffer::DescriptorType::CBV;

        mVCTMainCB = new CBuffer(cbDesc, L"VCT main CB");

        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        mVCTMainRT = new CRenderTarget(FRAME_BUFFER_WIDTH * mVCTRTRatio, FRAME_BUFFER_HEIGHT * mVCTRTRatio, format, flags, L"VCT Final Output", -1, 1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        //create root signature
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;


        // PS
        {
            mVCTMainRS.Reset(2, 1);
            mVCTMainRS.InitStaticSampler(0, mBilinearSamplerClamp, D3D12_SHADER_VISIBILITY_ALL);
            mVCTMainRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);
            mVCTMainRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_ALL);
            mVCTMainRS.Finalize(device, L"VCT main pass pixel version RS", rootSignatureFlags);

            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif

            ID3DBlob* errorBlob = nullptr;

            DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vertexShader, &errorBlob));
            if (errorBlob)
            {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingPS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, &errorBlob));
            if (errorBlob)
            {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
               { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
               { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            DXGI_FORMAT formats[1];
            formats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

            mVCTMainPSO.SetRootSignature(mVCTMainRS);
            mVCTMainPSO.SetRasterizerState(mRasterizerState);
            mVCTMainPSO.SetBlendState(mBlendState);
            mVCTMainPSO.SetDepthStencilState(mDepthStateDisabled);
            mVCTMainPSO.SetInputLayout(_countof(inputElementDescs), inputElementDescs);
            mVCTMainPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
            mVCTMainPSO.SetRenderTargetFormats(_countof(formats), formats, DXGI_FORMAT_D32_FLOAT);
            mVCTMainPSO.SetVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize());
            mVCTMainPSO.SetPixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize());
            mVCTMainPSO.Finalize(device);
        }

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
//            DX::ThrowIfFailed(D3DCompileFromFile(L"C:\\directX_work\\VoteFight_new\\Release\\Asset\\Shader\\VoxelConeTracingCS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &computeShader, &errorBlob));
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
    }

}

void VCT::RenderObject(std::unique_ptr<CObject>& aModel, std::function<void(std::unique_ptr<CObject>&)> aCallback)
{
    if (aCallback) {
        aCallback(aModel);
    }
}

void VCT::RenderVoxelConeTracing()
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
    DescriptorHeapManager* descriptorManager = framework->GetDescriptorHeapManager();
    GPUDescriptorHeap* gpuDescriptorHeap = descriptorManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    RECT Winrect;
    GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &Winrect);

    CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, Winrect.right, Winrect.bottom);
    CD3DX12_RECT rect = CD3DX12_RECT(0.0f, 0.0f, Winrect.right, Winrect.bottom);

    auto clearVCTMainRT = [this, commandList]() {
        commandList->SetPipelineState(mVCTMainPSO.GetPipelineStateObject());
        commandList->SetGraphicsRootSignature(mVCTMainRS.GetSignature());

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesRSM[] = { mVCTMainRT->GetRTV().GetCPUHandle() };
        D3D12_CPU_DESCRIPTOR_HANDLE uavHandlesRSM[] = { mVCTMainUpsampleAndBlurRT->GetUAV().GetCPUHandle() };

        CGameFramework* framework = CGameFramework::GetInstance();
        //transition buffers to rendertarget outputs
        framework->ResourceBarriersBegin(mBarriers);
        mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        framework->ResourceBarriersEnd(mBarriers, commandList);

        commandList->OMSetRenderTargets(_countof(rtvHandlesRSM), rtvHandlesRSM, FALSE, nullptr);
        commandList->ClearRenderTargetView(rtvHandlesRSM[0], clearColorBlack, 0, nullptr);
        };

    // VCT Voxelization
    {
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

        DescriptorHandle cbvHandle;
        DescriptorHandle uavHandle;
        DescriptorHandle srvHandle;
        uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTVoxelization3DRT->GetUAV());

        commandList->ClearUnorderedAccessViewFloat(uavHandle.GetGPUHandle(), mVCTVoxelization3DRT->GetUAV().GetCPUHandle(), mVCTVoxelization3DRT->GetResource(), clearColorBlack, 0, nullptr);

        srvHandle = gpuDescriptorHeap->GetHandleBlock(1);

        // gpuDescriptorHeap->AddToHandle(device, srvHandle, mShadowDepth->GetSRV());


        for (auto& model : mRenderableObjects) {
            RenderObject(model, [this, gpuDescriptorHeap, commandList, &cbvHandle, &uavHandle, &srvHandle, device](std::unique_ptr<CObject>& anObject) {
                cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
                gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());

                __declspec(align(16)) struct ModelConstantBuffer
                {
                    XMMATRIX	World;
                    XMFLOAT4	DiffuseColor;
                };

                CBuffer::Description desc;
                desc.mElementSize = sizeof(ModelConstantBuffer);
                desc.mState = D3D12_RESOURCE_STATE_GENERIC_READ;
                desc.mDescriptorType = CBuffer::DescriptorType::CBV;

                CBuffer* mBufferCB = new CBuffer(desc, L"Model CB");

                ModelConstantBuffer cbData = {};
                cbData.World = XMMatrixIdentity();
                cbData.DiffuseColor = XMFLOAT4(1, 0, 0, 0);
                memcpy(mBufferCB->Map(), &cbData, sizeof(cbData));

                gpuDescriptorHeap->AddToHandle(device, cbvHandle, mBufferCB->GetCBV());

                commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
                commandList->SetGraphicsRootDescriptorTable(1, srvHandle.GetGPUHandle());
                commandList->SetGraphicsRootDescriptorTable(2, uavHandle.GetGPUHandle());

                //anObject->Render();

                commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                CMesh* mesh = anObject->GetMesh();
                mesh->Render(0);
                });
        }

        //reset back
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &rect);
    }

    // Voxelization Debug
    {
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

        DescriptorHandle cbvHandle;
        DescriptorHandle uavHandle;

        cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());

        uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTVoxelization3DRT->GetUAV());

        commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
        commandList->SetGraphicsRootDescriptorTable(1, uavHandle.GetGPUHandle());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        commandList->DrawInstanced(VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE, 1, 0, 0);
    }

//    // [Debug] Render DepthTexture
//    const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();
//    D3D12_VIEWPORT d3d12Viewport = { 0.0f, 0.0f, resolution.x * 0.4f, resolution.y * 0.4f, 0.0f, 1.0f };
//    D3D12_RECT d3d12ScissorRect = { 0, 0,(LONG)(resolution.x * 0.4f), (LONG)(resolution.y * 0.4f) };
//    CShader* shader = CAssetManager::GetInstance()->GetShader("DepthWrite");
//
//    commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(ROOT_PARAMETER_TYPE::SHADOW_MAP), mVCTVoxelizationDebugRT->GetRTV().GetGPUHandle());
//    shader->SetPipelineState(2);
//    commandList->RSSetViewports(1, &d3d12Viewport);
//    commandList->RSSetScissorRects(1, &d3d12ScissorRect);
//    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    commandList->DrawInstanced(6, 1, 0, 0);

    // VCT Mipmapping prepare CS
    {
        commandList->SetPipelineState(mVCTAnisoMipmappingPreparePSO.GetPipelineStateObject());
        commandList->SetComputeRootSignature(mVCTAnisoMipmappingPrepareRS.GetSignature());

        framework->ResourceBarriersBegin(mBarriers);
        mVCTAnisoMipmappinPrepare3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        framework->ResourceBarriersEnd(mBarriers, commandList);

        VCTAnisoMipmappingCBData cbData = {};
        cbData.MipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
        cbData.MipLevel = 0;
        ::memcpy(mVCTAnisoMipmappingCB->Map(), &cbData, sizeof(cbData));
        DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTAnisoMipmappingCB->GetCBV());

        DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT_CopyForAsync->GetSRV());
        // gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT->GetSRV());

        DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(6);
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[0]->GetUAV());
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[1]->GetUAV());
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[2]->GetUAV());
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[3]->GetUAV());
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[4]->GetUAV());
        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[5]->GetUAV());

        commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
        commandList->SetComputeRootDescriptorTable(1, srvHandle.GetGPUHandle());
        commandList->SetComputeRootDescriptorTable(2, uavHandle.GetGPUHandle());

        commandList->Dispatch(DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u));
    }

    // VCT Mipmapping main CS
    {
        commandList->SetPipelineState(mVCTAnisoMipmappingMainPSO.GetPipelineStateObject());
        commandList->SetComputeRootSignature(mVCTAnisoMipmappingMainRS.GetSignature());

        framework->ResourceBarriersBegin(mBarriers);
        mVCTAnisoMipmappinPrepare3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinPrepare3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        mVCTAnisoMipmappinMain3DRTs[0]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[1]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[2]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[3]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[4]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        mVCTAnisoMipmappinMain3DRTs[5]->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        framework->ResourceBarriersEnd(mBarriers, commandList);

        int mipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
        for (int mip = 0; mip < VCT_MIPS; mip++)
        {
            DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
            gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTAnisoMipmappingMainCB[mip]->GetCBV());

            DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(12);
            if (mip == 0) {
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[0]->GetUAV());
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[1]->GetUAV());
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[2]->GetUAV());
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[3]->GetUAV());
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[4]->GetUAV());
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[5]->GetUAV());
            }
            else {
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetUAV(mip - 1));
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetUAV(mip - 1));
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetUAV(mip - 1));
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetUAV(mip - 1));
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetUAV(mip - 1));
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetUAV(mip - 1));
            }
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetUAV(mip));
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetUAV(mip));
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetUAV(mip));
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetUAV(mip));
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetUAV(mip));
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetUAV(mip));

            VCTAnisoMipmappingCBData cbData = {};
            cbData.MipDimension = mipDimension;
            cbData.MipLevel = mip;
            memcpy(mVCTAnisoMipmappingMainCB[mip]->Map(), &cbData, sizeof(cbData));
            commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
            commandList->SetComputeRootDescriptorTable(1, uavHandle.GetGPUHandle());

            commandList->Dispatch(DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u), DivideByMultiple(static_cast<UINT>(cbData.MipDimension), 8u));
            mipDimension >>= 1;
        }
    }

    // VCT Main PS
    {
        CD3DX12_VIEWPORT vctResViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, mVCTMainRT->GetWidth(), mVCTMainRT->GetHeight());
        CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, mVCTMainRT->GetWidth(), mVCTMainRT->GetHeight());
        commandList->RSSetViewports(1, &vctResViewport);
        commandList->RSSetScissorRects(1, &vctRect);

        commandList->SetPipelineState(mVCTMainPSO.GetPipelineStateObject());
        commandList->SetGraphicsRootSignature(mVCTMainRS.GetSignature());

        framework->ResourceBarriersBegin(mBarriers);
        mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        framework->ResourceBarriersEnd(mBarriers, commandList);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesFinal[] =
        {
            mVCTMainRT->GetRTV().GetCPUHandle()
        };

        commandList->OMSetRenderTargets(_countof(rtvHandlesFinal), rtvHandlesFinal, FALSE, nullptr);
        commandList->ClearRenderTargetView(rtvHandlesFinal[0], clearColorBlack, 0, nullptr);

        DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(10);
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[0]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[1]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[2]->GetSRV());

        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetSRV());
        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT->GetSRV());

        DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());
        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTMainCB->GetCBV());

        commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
        commandList->SetGraphicsRootDescriptorTable(1, srvHandle.GetGPUHandle());

        commandList->IASetVertexBuffers(0, 1, &framework->GetFullscreenQuadBufferView());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        commandList->DrawInstanced(4, 1, 0, 0);

        //reset back
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &rect);
    }

// VCT Main CS
//    {
//        commandList->SetPipelineState(mVCTMainPSO_Compute.GetPipelineStateObject());
//        commandList->SetComputeRootSignature(mVCTMainRS_Compute.GetSignature());
//
//        framework->ResourceBarriersBegin(mBarriers);
//        mVCTMainRT->TransitionTo(mBarriers, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//        framework->ResourceBarriersEnd(mBarriers, commandList);
//
//        DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
//        gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTMainRT->GetUAV());
//
//        DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(10);
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[0]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[1]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mGbufferRTs[2]->GetSRV());
//
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[0]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[1]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[2]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[3]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[4]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTAnisoMipmappinMain3DRTs[5]->GetSRV());
//        gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT_CopyForAsync->GetSRV());
//        // gpuDescriptorHeap->AddToHandle(device, srvHandle, mVCTVoxelization3DRT->GetSRV());
//
//        DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
//        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTVoxelizationCB->GetCBV());
//        gpuDescriptorHeap->AddToHandle(device, cbvHandle, mVCTMainCB->GetCBV());
//
//        commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
//        commandList->SetComputeRootDescriptorTable(1, srvHandle.GetGPUHandle());
//        commandList->SetComputeRootDescriptorTable(2, uavHandle.GetGPUHandle());
//
//        commandList->Dispatch(DivideByMultiple(static_cast<UINT>(mVCTMainRT->GetWidth()), 8u), DivideByMultiple(static_cast<UINT>(mVCTMainRT->GetHeight()), 8u), 1u);
//    }
}