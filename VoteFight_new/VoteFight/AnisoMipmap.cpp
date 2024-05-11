#include "pch.h"
#include "GameFramework.h"
#include "DescriptorHeap.h"
#include "Texture.h"
#include "AssetManager.h"
#include "VoxelizationShader.h"
#include "AnisoMipmap.h"

CAnisoMipmap::CAnisoMipmap()
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

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavPrepareDesc = {};
        uavPrepareDesc.Format = format;
        uavPrepareDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavPrepareDesc.Texture3D.MipSlice = 0;
        uavPrepareDesc.Texture3D.WSize = size;

        for (int i = 0; i < 6; ++i) {
            mVCTAnisoMipmappinPrepare3DRTs[i] = new Texture3D();
            mVCTAnisoMipmappinPrepare3DRTs[i]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size);
            mVCTAnisoMipmappinPrepare3DRTs[i]->m_UAVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            device->CreateUnorderedAccessView(mVCTAnisoMipmappinPrepare3DRTs[i]->GetTexture(), nullptr, &uavPrepareDesc, mVCTAnisoMipmappinPrepare3DRTs[i]->m_UAVHandle.GetCPUHandle());
            mVCTAnisoMipmappinMain3DRTs[i] = new Texture3D();
            mVCTAnisoMipmappinMain3DRTs[i]->Create(static_cast<UINT64>(size), static_cast<UINT>(size), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, size, VCT_MIPS);
        }
    }
    UINT bytes = (sizeof(CB_ANISO_MIPMAP) + 255) & ~255;
    
    m_AnisoMipmapBufferPrepare = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
    DX::ThrowIfFailed(m_AnisoMipmapBufferPrepare->Map(0, nullptr, reinterpret_cast<void**>(&m_AnisoMipmapMappedDataPrepare)));

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.SizeInBytes = bytes;
    cbvDesc.BufferLocation = m_AnisoMipmapBufferPrepare->GetGPUVirtualAddress();

    m_cpuAnisoMipmapPrepare = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateConstantBufferView(&cbvDesc, m_cpuAnisoMipmapPrepare.GetCPUHandle());
    
    m_AnisoMipmapBuffers.resize(VCT_MIPS);
    m_cpuAnisoMipmaps.resize(VCT_MIPS);
    m_AnisoMipmapMappedDatas.resize(VCT_MIPS);

    for (int i = 0; i < VCT_MIPS; ++i) {
        m_AnisoMipmapBuffers[i] = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
        DX::ThrowIfFailed(m_AnisoMipmapBuffers[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_AnisoMipmapMappedDatas[i])));
        cbvDesc.BufferLocation = m_AnisoMipmapBuffers[i]->GetGPUVirtualAddress();
        m_cpuAnisoMipmaps[i] = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        device->CreateConstantBufferView(&cbvDesc, m_cpuAnisoMipmaps[i].GetCPUHandle());
    }
}

CAnisoMipmap::~CAnisoMipmap()
{
}

D3D12_SHADER_BYTECODE CAnisoMipmap::CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
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

ID3D12RootSignature* CAnisoMipmap::CreateComputeRootSignature(int stateNum)
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
    case 0:
    {
        // prepare
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
        // Main
        CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
        CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[2] = {};
        d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 12, 0);
        CD3DX12_ROOT_PARAMETER d3d12RootParameters[2] = {};
        d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
        d3d12RootParameters[1].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
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
    }
    return m_rootSignature[stateNum];
}

void CAnisoMipmap::Render(int stateNum)
{
    CGameFramework* framework = CGameFramework::GetInstance();
    ID3D12Device* device = framework->GetDevice();
    ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
    DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
    GPUDescriptorHeap* gpuDescriptorHeap = descriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    commandList->SetComputeRootSignature(m_rootSignature[stateNum]);

    switch (stateNum) {
    case 0:
    {
        int mipdemension = VCT_SCENE_VOLUME_SIZE >> 1;
        m_AnisoMipmapMappedDataPrepare->MipDimension = mipdemension;
        m_AnisoMipmapMappedDataPrepare->MipLevel = 0;

        // CBV
        DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        gpuDescriptorHeap->AddToHandle(device, cbvHandle, m_cpuAnisoMipmapPrepare);
        commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());

        // SRV
        DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(1);
        CVoxelizationShader* pShader = reinterpret_cast<CVoxelizationShader*>(CAssetManager::GetInstance()->GetShader("Voxelization"));
        Texture3D* voxelizationTexture = pShader->GetVoxelTexture();
        gpuDescriptorHeap->AddToHandle(device, srvHandle, voxelizationTexture->m_SRVHandle);
        commandList->SetComputeRootDescriptorTable(1, srvHandle.GetGPUHandle());

        // UAV
        for (int i = 0; i < 6; ++i) {
            DX::ResourceTransition(commandList, mVCTAnisoMipmappinPrepare3DRTs[i]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        }

        DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(6);
        for (int i = 0; i < 6; ++i) {
            gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[i]->m_UAVHandle);
        }
        commandList->SetComputeRootDescriptorTable(2, uavHandle.GetGPUHandle());

        for (int i = 0; i < 6; ++i) {
            DX::ResourceTransition(commandList, mVCTAnisoMipmappinPrepare3DRTs[i]->GetTexture(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
        }

        SetThreadGroup(XMUINT3(DivideByMultiple(static_cast<UINT>(mipdemension), 8u), DivideByMultiple(static_cast<UINT>(mipdemension), 8u), DivideByMultiple(static_cast<UINT>(mipdemension), 8u)));
        Dispatch(stateNum);

    }
    break;
    case 1:
    {
        // for (int i = 0; i < 6; ++i) {
        //     DX::ResourceTransition(commandList, mVCTAnisoMipmappinPrepare3DRTs[i]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        //     DX::ResourceTransition(commandList, mVCTAnisoMipmappinMain3DRTs[i]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        // }

        int mipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
        for (int mip = 0; mip < VCT_MIPS; mip++)
        {
            DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(1);
            gpuDescriptorHeap->AddToHandle(device, cbvHandle, m_cpuAnisoMipmaps[mip]);

            DescriptorHandle uavHandle = gpuDescriptorHeap->GetHandleBlock(12);
            if (mip == 0) {
                for (int i = 0; i < 6; ++i) {
                    gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinPrepare3DRTs[i]->m_UAVHandle);
                }
            }
            else {
                for (int i = 0; i < 6; ++i) {
                    gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[i]->m_UAVHandles[mip - 1]);
                }
            }
            for (int i = 0; i < 6; ++i) {
                gpuDescriptorHeap->AddToHandle(device, uavHandle, mVCTAnisoMipmappinMain3DRTs[i]->m_UAVHandles[mip]);
            }

            // CBV
            m_AnisoMipmapMappedDatas[mip]->MipDimension = mipDimension;
            m_AnisoMipmapMappedDatas[mip]->MipLevel = mip;

            commandList->SetComputeRootDescriptorTable(0, cbvHandle.GetGPUHandle());
            commandList->SetComputeRootDescriptorTable(1, uavHandle.GetGPUHandle());

            SetThreadGroup(XMUINT3(DivideByMultiple(static_cast<UINT>(mipDimension), 8u), DivideByMultiple(static_cast<UINT>(mipDimension), 8u), DivideByMultiple(static_cast<UINT>(mipDimension), 8u)));
            Dispatch(stateNum);
            mipDimension >>= 1;
        }
    }
    break;
    }

}
