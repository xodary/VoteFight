#include "pch.h"
#include "UpsampleBlur.h"
#include "AssetManager.h"
#include "VCTMainShader.h"
#include "GameFramework.h"
#include "DescriptorHeap.h"

CUpsampleBlur::CUpsampleBlur()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
	GPUDescriptorHeap* gpuDescriptorHeap = descriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	m_UpsampleBlurRT = new Texture3D();

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_UpsampleBlurRT->Create(static_cast<UINT64>(FRAME_BUFFER_WIDTH), static_cast<UINT>(FRAME_BUFFER_HEIGHT), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, flags, format, D3D12_CLEAR_VALUE{ format, { 1.0f, 1.0f, 1.0f, 1.0f } }, TEXTURE_TYPE::ALBEDO_MAP);
	m_UpsampleBlurRT->m_UAVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateUnorderedAccessView(m_UpsampleBlurRT->GetTexture(), nullptr, &uavDesc, m_UpsampleBlurRT->m_UAVHandle.GetCPUHandle());

}

CUpsampleBlur::~CUpsampleBlur()
{
}

ID3D12RootSignature* CUpsampleBlur::CreateComputeRootSignature(int stateNum)
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
    CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[2] = {};
    d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    CD3DX12_ROOT_PARAMETER d3d12RootParameters[2] = {};
    d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
    d3d12RootParameters[1].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
	CD3DX12_STATIC_SAMPLER_DESC d3d12SamplerDesc[1] = {};
	d3d12SamplerDesc[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_ALL);
	d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, _countof(d3d12SamplerDesc), d3d12SamplerDesc, rootSignatureFlags);
    ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
    if (d3d12ErrorBlob) {
        char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
        std::cerr << "VCT - Direct3D Error: " << pErrorString << std::endl;
    }
    DX::ThrowIfFailed(device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignature)));

    return m_rootSignature;
}

D3D12_SHADER_BYTECODE CUpsampleBlur::CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("UpsampleBlurCS.hlsl", "CS_Main", "cs_5_1", d3d12ShaderBlob);
}

void CUpsampleBlur::Render(int stateNum)
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
	GPUDescriptorHeap* gpuDescriptorHeap = descriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	SetPipelineState(0);
	commandList->SetComputeRootSignature(m_rootSignature);

	// DX::ResourceTransition(commandList, m_UpsampleBlurRT->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	DescriptorHandle srvHandleBlur = gpuDescriptorHeap->GetHandleBlock(1);
	CVCTMainShader* pVCTMainShader = reinterpret_cast<CVCTMainShader*>(CAssetManager::GetInstance()->GetShader("VCTMain"));
	CD3DX12_CPU_DESCRIPTOR_HANDLE  D3D12RtvCPUDescriptorHandle(framework->GetRtvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());
	//gpuDescriptorHeap->AddToHandle(device, srvHandleBlur, pVCTMainShader->m_VCTMainRT->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandleBlur, framework->SRVHandle[framework->m_swapChainBufferIndex]);

	DescriptorHandle uavHandleBlur = gpuDescriptorHeap->GetHandleBlock(1);
	gpuDescriptorHeap->AddToHandle(device, uavHandleBlur, m_UpsampleBlurRT->m_UAVHandle);

	commandList->SetComputeRootDescriptorTable(0, srvHandleBlur.GetGPUHandle());
	commandList->SetComputeRootDescriptorTable(1, uavHandleBlur.GetGPUHandle());
	
	SetThreadGroup(XMUINT3(DivideByMultiple(static_cast<UINT>(FRAME_BUFFER_WIDTH), 8u), DivideByMultiple(static_cast<UINT>(FRAME_BUFFER_HEIGHT), 8u), 1u));
	Dispatch(0);
}

