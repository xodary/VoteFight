#include "pch.h"
#include "AnisoMipmap.h"
#include "GameFramework.h"
#include "DescriptorHeap.h"
#include "Texture.h"

AnisoMipmap::AnisoMipmap()
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
    DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();

    // Voxelization 3D Render Target 텍스쳐 생성
    {
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        int size = VCT_SCENE_VOLUME_SIZE >> 1;

        mVCTAnisoMipmappinPrepare3DRTs[0] = new Texture3D();
        mVCTAnisoMipmappinPrepare3DRTs[1] = new Texture3D();
        mVCTAnisoMipmappinPrepare3DRTs[2] = new Texture3D();
        mVCTAnisoMipmappinPrepare3DRTs[3] = new Texture3D();
        mVCTAnisoMipmappinPrepare3DRTs[4] = new Texture3D();
        mVCTAnisoMipmappinPrepare3DRTs[5] = new Texture3D();

        mVCTAnisoMipmappinPrepare3DRTs[0]->SetName("Voxelization_Scene_Mip_Prepare_X+_3D");
        mVCTAnisoMipmappinPrepare3DRTs[1]->SetName("Voxelization_Scene_Mip_Prepare_X-_3D");
        mVCTAnisoMipmappinPrepare3DRTs[2]->SetName("Voxelization_Scene_Mip_Prepare_Y+_3D");
        mVCTAnisoMipmappinPrepare3DRTs[3]->SetName("Voxelization_Scene_Mip_Prepare_Y-_3D");
        mVCTAnisoMipmappinPrepare3DRTs[4]->SetName("Voxelization_Scene_Mip_Prepare_Z+_3D");
        mVCTAnisoMipmappinPrepare3DRTs[5]->SetName("Voxelization_Scene_Mip_Prepare_Z-_3D");

        mVCTAnisoMipmappinPrepare3DRTs[0]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
        mVCTAnisoMipmappinPrepare3DRTs[1]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
        mVCTAnisoMipmappinPrepare3DRTs[2]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
        mVCTAnisoMipmappinPrepare3DRTs[3]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
        mVCTAnisoMipmappinPrepare3DRTs[4]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
        mVCTAnisoMipmappinPrepare3DRTs[5]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
    }

    UINT bytes = (sizeof(CB_ANISO_MIPMAP_PREPARE) + 255) & ~255;
    m_AnisoMipmapPrepareBuffer = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
    DX::ThrowIfFailed(m_AnisoMipmapPrepareBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_AnisoMipmapPrepareMappedData)));
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.SizeInBytes = bytes;
    cbvDesc.BufferLocation = m_AnisoMipmapPrepareBuffer->GetGPUVirtualAddress();
    m_cpuAnisoMipmapPrepare = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateConstantBufferView(&cbvDesc, m_cpuAnisoMipmapPrepare.GetCPUHandle());
}

AnisoMipmap::~AnisoMipmap()
{
}

D3D12_SHADER_BYTECODE AnisoMipmap::CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
    switch (stateNum)
    {
    case 0: // AnisoMipmapPrepare
        return Compile("VoxelConeTracingAnisoMipmapPrepareCS.hlsl", "CS_Main", "cs_5_1", d3d12ShaderBlob);
    case 1: // AnisoMipmapMain
        return Compile("VoxelConeTracingAnisoMipmapMainCS.hlsl", "CS_Main", "cs_5_1", d3d12ShaderBlob);
    }

    return CComputeShader::CreateComputeShader(d3d12ShaderBlob, stateNum);
}

ID3D12RootSignature* AnisoMipmap::CreateRootSignature(int stateNum)
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    m_rootSignature[stateNum] = {};
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    switch (stateNum) {
        // prepare
    case 0:
    {
        CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
        CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[3] = {};
        d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        d3d12DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 6, 0);
        CD3DX12_ROOT_PARAMETER d3d12RootParameters[3] = {};
        d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
        d3d12RootParameters[1].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
        d3d12RootParameters[2].InitAsDescriptorTable(1, &d3d12DescriptorRanges[2]);
        d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, 0, nullptr, rootSignatureFlags);
        ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;
        DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
        if (d3d12ErrorBlob) {
            char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
            std::cerr << "VCT - Direct3D Error: " << pErrorString << std::endl;
        }
        DX::ThrowIfFailed(device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignature[stateNum])));
    }
    break;
    case 1: 
    {

    }
    break;
    }
}

void AnisoMipmap::UpdateShaderVariables()
{
}

void AnisoMipmap::Render(int stateNum)
{
}
