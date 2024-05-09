#include "pch.h"
#include "Shader.h"
#include "Texture.h"
#include "GameFramework.h"
#include "VoxelizationShader.h"
#include "AssetManager.h"
#include "Camera.h"
#include "CameraManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Mesh.h"
#include "Transform.h"
#include "GameScene.h"

CVoxelizationShader::CVoxelizationShader()
{
    ID3D12Device* device = CGameFramework::GetInstance()->GetDevice();
    ID3D12GraphicsCommandList* commandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

    // Voxelization 3D Render Target 咆胶媚 积己
    {
        m_VCTVoxelization3DRT = new VCTTexture();
        mVCTVoxelizationDebugRT = new VCTTexture();

        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        m_VCTVoxelization3DRT->SetName("Voxelization");
        mVCTVoxelizationDebugRT->SetName("VoxelizationDebug");
        
        m_VCTVoxelization3DRT->Create(static_cast<UINT64>(VCT_SCENE_VOLUME_SIZE), static_cast<UINT>(VCT_SCENE_VOLUME_SIZE), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, 2, VCT_SCENE_VOLUME_SIZE);
        mVCTVoxelizationDebugRT->Create(static_cast<UINT64>(FRAME_BUFFER_WIDTH), static_cast<UINT>(FRAME_BUFFER_HEIGHT), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, 2, VCT_SCENE_VOLUME_SIZE);
    }
    
    // Descriptor Heap 积己
    {
        m_DescriptorHeapManager = new DescriptorHeapManager();

        D3D12_DESCRIPTOR_HEAP_DESC D3D12DescriptorHeapDesc = {};

        // RTV
        D3D12DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        D3D12DescriptorHeapDesc.NumDescriptors = 1;
        D3D12DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        D3D12DescriptorHeapDesc.NodeMask = 0;
        DX::ThrowIfFailed(device->CreateDescriptorHeap(&D3D12DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&RtvDescriptorHeap)));

        // DSV
        D3D12DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        D3D12DescriptorHeapDesc.NumDescriptors = 1;
        DX::ThrowIfFailed(device->CreateDescriptorHeap(&D3D12DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&DsvDescriptorHeap)));
    }

    // CBV 积己
    {
        UINT bytes = (sizeof(CB_VOXELIZATION) + 255) & ~255;

        m_VoxelizationBuffer = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
        DX::ThrowIfFailed(m_VoxelizationBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_VoxelizationMappedData)));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.SizeInBytes = bytes;
        cbvDesc.BufferLocation = m_VoxelizationBuffer->GetGPUVirtualAddress();
        m_cpuVoxelization = m_DescriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        device->CreateConstantBufferView(&cbvDesc, m_cpuVoxelization.GetCPUHandle());

        bytes = (sizeof(CB_MODEL) + 255) & ~255;

        m_ModelBuffer = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
        DX::ThrowIfFailed(m_ModelBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_ModelMappedData)));

        cbvDesc.SizeInBytes = bytes;
        cbvDesc.BufferLocation = m_ModelBuffer->GetGPUVirtualAddress();
        m_cpuModel = m_DescriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        device->CreateConstantBufferView(&cbvDesc, m_cpuModel.GetCPUHandle());

    }
    
    // SRV 积己
    {
    }

    // Texture狼 RTV 积己
    {
        D3D12_CPU_DESCRIPTOR_HANDLE RtvCpuDescriptorHandle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice = 0;
        rtvDesc.Texture3D.WSize = (VCT_SCENE_VOLUME_SIZE >> 0);

        device->CreateRenderTargetView(m_VCTVoxelization3DRT->GetTexture(), &rtvDesc, RtvCpuDescriptorHandle);
        RtvCpuDescriptorHandle.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Texture狼 UAV 积己
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.WSize = (VCT_SCENE_VOLUME_SIZE >> 0);

        device->CreateUnorderedAccessView(m_VCTVoxelization3DRT->GetTexture(), nullptr, &uavDesc, m_VCTVoxelization3DRT->GetCPUHandle().GetCPUHandle());
    }

    //
}

CVoxelizationShader::~CVoxelizationShader()
{
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE CVoxelizationShader::GetPrimitiveType(int stateNum)
{
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

ID3D12RootSignature* CVoxelizationShader::CreateRootSignature(int stateNum)
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    rootSignature = {};
    CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    switch (stateNum) {
    case 0:
    {
        CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[4] = {};
        d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
        d3d12DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        d3d12DescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        CD3DX12_ROOT_PARAMETER d3d12RootParameters[4] = {};
        d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
        d3d12RootParameters[1].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
        d3d12RootParameters[2].InitAsDescriptorTable(1, &d3d12DescriptorRanges[2]);
        d3d12RootParameters[3].InitAsDescriptorTable(1, &d3d12DescriptorRanges[3]);
        CD3DX12_STATIC_SAMPLER_DESC d3d12SamplerDesc[1] = {};
        d3d12SamplerDesc[0].Init(0, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL);
        d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, _countof(d3d12SamplerDesc), d3d12SamplerDesc, rootSignatureFlags);
    }
    break;
    case 1:
    {
        CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRangesDebug[2] = {};
        d3d12DescriptorRangesDebug[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        d3d12DescriptorRangesDebug[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
        CD3DX12_ROOT_PARAMETER d3d12RootParametersDebug[2] = {};
        d3d12RootParametersDebug[0].InitAsDescriptorTable(1, &d3d12DescriptorRangesDebug[0]);
        d3d12RootParametersDebug[1].InitAsDescriptorTable(1, &d3d12DescriptorRangesDebug[1]);
        d3d12RootSignatureDesc.Init(_countof(d3d12RootParametersDebug), d3d12RootParametersDebug, 0, nullptr, rootSignatureFlags);
    }
    break;
    }
    // Voxelization Root Signature 积己
    ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
    if (d3d12ErrorBlob) {
        char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
        std::cerr << "VCT - Direct3D Error: " << pErrorString << std::endl;
    }
    DX::ThrowIfFailed(device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&rootSignature)));

    return rootSignature;
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
        break;
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
        break;
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
        break;
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

void CVoxelizationShader::UpdateShaderVariables()
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12GraphicsCommandList* commandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
    
    std::vector<CCamera*> cameras = CCameraManager::GetInstance()->GetCameras();
    XMMATRIX mCameraView = XMLoadFloat4x4(&cameras[0]->GetViewMatrix());
    XMMATRIX mCameraProjection = XMLoadFloat4x4(&cameras[0]->GetProjectionMatrix());
    XMMATRIX viewprojection = mCameraView * mCameraProjection;
    XMMATRIX mLightViewProjection = XMLoadFloat4x4(&cameras[2]->GetViewMatrix()) * XMLoadFloat4x4(&cameras[2]->GetProjectionMatrix());

    XMStoreFloat4x4(&m_VoxelizationMappedData->WorldVoxelCube, XMMatrixScaling(1.0f, 1.0f, 1.0f));
    XMStoreFloat4x4(&m_VoxelizationMappedData->ViewProjection, viewprojection);
    XMStoreFloat4x4(&m_VoxelizationMappedData->ShadowViewProjection, mLightViewProjection);
    float mWorldVoxelScale = VCT_SCENE_VOLUME_SIZE * 0.5f;
    memcpy(&m_VoxelizationMappedData->WorldVoxelScale, &mWorldVoxelScale, sizeof(float));

    CObject* object = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::STRUCTURE)[0];
    CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
    XMStoreFloat4x4(&m_ModelMappedData->World, XMLoadFloat4x4(&transform->GetWorldMatrix()));
    XMFLOAT4 f4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    memcpy(&m_ModelMappedData->DiffuseColor, &f4, sizeof(XMFLOAT4));
}

void CVoxelizationShader::Render(int stateNum)
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
    GPUDescriptorHeap* gpuDescriptorHeap = m_DescriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    ID3D12DescriptorHeap* ppHeaps = nullptr;

    switch (stateNum) {
    case 0:
    {
        gpuDescriptorHeap->Reset();
        ppHeaps = gpuDescriptorHeap->GetHeap();

        CD3DX12_VIEWPORT vctViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE);
        CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, VCT_SCENE_VOLUME_SIZE, VCT_SCENE_VOLUME_SIZE);
        commandList->RSSetViewports(1, &vctViewport);
        commandList->RSSetScissorRects(1, &vctRect);
        commandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);

        SetPipelineState(0);
        commandList->SetGraphicsRootSignature(rootSignature);
        commandList->SetDescriptorHeaps(1, &ppHeaps);

        // CBV
        // UpdateShaderVariables();
        std::vector<CCamera*> cameras = CCameraManager::GetInstance()->GetCameras();
        XMMATRIX mCameraView = XMLoadFloat4x4(&cameras[0]->GetViewMatrix());
        XMMATRIX mCameraProjection = XMLoadFloat4x4(&cameras[0]->GetProjectionMatrix());
        XMMATRIX viewprojection = mCameraView * mCameraProjection;
        XMMATRIX mLightViewProjection = XMLoadFloat4x4(&cameras[2]->GetViewMatrix()) * XMLoadFloat4x4(&cameras[2]->GetProjectionMatrix());

        XMStoreFloat4x4(&m_VoxelizationMappedData->WorldVoxelCube, XMMatrixScaling(1.0f, 1.0f, 1.0f));
        XMStoreFloat4x4(&m_VoxelizationMappedData->ViewProjection, viewprojection);
        XMStoreFloat4x4(&m_VoxelizationMappedData->ShadowViewProjection, mLightViewProjection);
        float mWorldVoxelScale = VCT_SCENE_VOLUME_SIZE * 0.5f;
        memcpy(&m_VoxelizationMappedData->WorldVoxelScale, &mWorldVoxelScale, sizeof(float));

        CObject* object = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::STRUCTURE)[0];
        CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
        XMStoreFloat4x4(&m_ModelMappedData->World, XMLoadFloat4x4(&transform->GetWorldMatrix()));
        XMFLOAT4 f4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        memcpy(&m_ModelMappedData->DiffuseColor, &f4, sizeof(XMFLOAT4));

        // CBV
        DescriptorHandle cbv1Handle = gpuDescriptorHeap->GetHandleBlock(1);
        DescriptorHandle cbv2Handle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, cbv1Handle, m_cpuVoxelization);
        gpuDescriptorHeap->AddToHandle(device, cbv2Handle, m_cpuModel);
        commandList->SetGraphicsRootDescriptorTable(0, cbv1Handle.GetGPUHandle());
        commandList->SetGraphicsRootDescriptorTable(1, cbv2Handle.GetGPUHandle());

        // SRV
        DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, srvHandle, CAssetManager::GetInstance()->GetTexture("DepthWrite")->GetCPUHandle());
        commandList->SetGraphicsRootDescriptorTable(2, srvHandle.GetGPUHandle());

        // UAV
        DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
        DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        gpuDescriptorHeap->AddToHandle(device, uavHandle, m_VCTVoxelization3DRT->GetCPUHandle());
        float clearColorBlack[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        commandList->ClearUnorderedAccessViewFloat(uavHandle.GetGPUHandle(), m_VCTVoxelization3DRT->GetCPUHandle().GetCPUHandle(), m_VCTVoxelization3DRT->GetTexture(), clearColorBlack, 0, nullptr);

        commandList->SetGraphicsRootDescriptorTable(3, uavHandle.GetGPUHandle());

        object->GetMesh()->Render(0);

        DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
    }
    return;
    case 1:
    {
        CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        CD3DX12_RECT rect = CD3DX12_RECT(0.0f, 0.0f, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &rect);

        gpuDescriptorHeap->Reset();
        ppHeaps = gpuDescriptorHeap->GetHeap();

        CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
        XMMATRIX mCameraView = XMLoadFloat4x4(&camera->GetViewMatrix());
        XMMATRIX mCameraProjection = XMLoadFloat4x4(&camera->GetProjectionMatrix());
        XMMATRIX viewprojection = mCameraView * mCameraProjection;

        XMStoreFloat4x4(&m_VoxelizationMappedData->WorldVoxelCube, XMMatrixScaling(1.0f, 1.0f, 1.0f));
        XMStoreFloat4x4(&m_VoxelizationMappedData->ViewProjection, viewprojection);
        float mWorldVoxelScale = VCT_SCENE_VOLUME_SIZE * 0.5f;
        memcpy(&m_VoxelizationMappedData->WorldVoxelScale, &mWorldVoxelScale, sizeof(float));

        DescriptorHandle cbv1Handle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, cbv1Handle, m_cpuVoxelization);
        commandList->SetGraphicsRootDescriptorTable(0, cbv1Handle.GetGPUHandle());

        DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(1);
        DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        gpuDescriptorHeap->AddToHandle(device, uavHandle, m_VCTVoxelization3DRT->GetCPUHandle());
        commandList->SetGraphicsRootDescriptorTable(1, uavHandle.GetGPUHandle());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        commandList->DrawInstanced(VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE * VCT_SCENE_VOLUME_SIZE, 1, 0, 0);
        DX::ResourceTransition(commandList, m_VCTVoxelization3DRT->GetTexture(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
    }
    return;
    }
}

void CVoxelizationShader::VCTTexture::Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, int RS, UINT16 depth, UINT16 mips)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();

	rootSignature = RS;

	ComPtr<ID3D12Resource> texture = nullptr;
	CD3DX12_HEAP_PROPERTIES d3d12HeapProperties(D3D12_HEAP_TYPE_DEFAULT); //D3D12_HEAP_TYPE_READBACK
	CD3DX12_RESOURCE_DESC d3d12ResourceDesc = { D3D12_RESOURCE_DIMENSION_TEXTURE3D, 0, Width, Height, depth, mips, DxgiFormat, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12ResourceFlags };
    
	DX::ThrowIfFailed(d3d12Device->CreateCommittedResource(&d3d12HeapProperties, D3D12_HEAP_FLAG_NONE, &d3d12ResourceDesc, D3D12ResourceStates, &D3D12ClearValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(texture.GetAddressOf())));
    
    m_d3d12Texture = texture.Get();
}