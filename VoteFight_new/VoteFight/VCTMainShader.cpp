#include "pch.h"
#include "VCTMainShader.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "GBufferShader.h"
#include "AnisoMipmap.h"
#include "VoxelizationShader.h"
#include "Camera.h"
#include "Transform.h"
#include "CameraManager.h"

CVCTMainShader::CVCTMainShader()
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
	
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	m_VCTMainRT = new CTexture();
	m_VCTMainRT->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, flags, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::ALBEDO_MAP);
	m_VCTMainRT->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(m_VCTMainRT->GetTexture(), &rtvDesc, m_VCTMainRT->m_RTVHandle.GetCPUHandle());
	device->CreateShaderResourceView(m_VCTMainRT->GetTexture(), nullptr, m_VCTMainRT->m_SRVHandle.GetCPUHandle());

	UINT bytes = (sizeof(CB_VCT_MAIN) + 255) & ~255;
	m_VCTMainCBResource = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_VCTMainCBResource->Map(0, nullptr, reinterpret_cast<void**>(&m_VCTMainCBMappedData)));
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.SizeInBytes = bytes;
	cbvDesc.BufferLocation = m_VCTMainCBResource->GetGPUVirtualAddress();
	m_VCTMainCBCPUHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateConstantBufferView(&cbvDesc, m_VCTMainCBCPUHandle.GetCPUHandle());
}

CVCTMainShader::~CVCTMainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CVCTMainShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	inputElementCount = 2;
	d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;

}

ID3D12RootSignature* CVCTMainShader::CreateRootSignature(int stateNum)
{
	ID3D12Device* device = CGameFramework::GetInstance()->GetDevice();

	m_rootSignature = {};
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};
	CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[2] = {};
	d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);
	d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0);
	CD3DX12_ROOT_PARAMETER d3d12RootParameters[2] = {};
	d3d12RootParameters[0].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]);
	d3d12RootParameters[1].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]);
	CD3DX12_STATIC_SAMPLER_DESC d3d12SamplerDesc[1] = {};
	d3d12SamplerDesc[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL);
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

D3D12_SHADER_BYTECODE CVCTMainShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("VoxelConeTracingPS.hlsl", "VS_Main", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CVCTMainShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("VoxelConeTracingPS.hlsl", "PS_Main", "ps_5_1", d3d12ShaderBlob);
}

D3D12_RASTERIZER_DESC CVCTMainShader::CreateRasterizerState(int stateNum)
{
	D3D12_RASTERIZER_DESC rasterizerState = {};

	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerState.DepthClipEnable = TRUE;
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	rasterizerState.ForcedSampleCount = 0;
	rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerState;
}

D3D12_BLEND_DESC CVCTMainShader::CreateBlendState(int stateNum)
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

D3D12_DEPTH_STENCIL_DESC CVCTMainShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC depthStateDisabled = {};
	D3D12_DEPTH_STENCIL_DESC depthStateRW = {};

	depthStateRW.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStateRW.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

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

DXGI_FORMAT CVCTMainShader::GetRTVFormat(int stateNum)
{
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

void CVCTMainShader::Render(int stateNum)
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
	GPUDescriptorHeap* gpuDescriptorHeap = descriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CD3DX12_VIEWPORT vctResViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	CD3DX12_RECT vctRect = CD3DX12_RECT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	commandList->RSSetViewports(1, &vctResViewport);
	commandList->RSSetScissorRects(1, &vctRect);

	SetPipelineState(0);
	commandList->SetGraphicsRootSignature(m_rootSignature);

	DX::ResourceTransition(commandList, m_VCTMainRT->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandlesFinal[] =
	{
		 m_VCTMainRT->m_RTVHandle.GetCPUHandle()
	};

	float clearColorBlack[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->OMSetRenderTargets(_countof(rtvHandlesFinal), rtvHandlesFinal, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandlesFinal[0], clearColorBlack, 0, nullptr);

	DescriptorHandle srvHandle = gpuDescriptorHeap->GetHandleBlock(10);
	CGBufferShader* gbufferShader = reinterpret_cast<CGBufferShader*>(CAssetManager::GetInstance()->GetShader("GBuffer"));
	gpuDescriptorHeap->AddToHandle(device, srvHandle, gbufferShader->m_GBuffer[0]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, gbufferShader->m_GBuffer[1]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, gbufferShader->m_GBuffer[2]->m_SRVHandle);

	CAnisoMipmapShader* mipmapShader = reinterpret_cast<CAnisoMipmapShader*>(CAssetManager::GetInstance()->GetShader("AnisoMipmap"));
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[0]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[1]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[2]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[3]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[4]->m_SRVHandle);
	gpuDescriptorHeap->AddToHandle(device, srvHandle, mipmapShader->mVCTAnisoMipmappinMain3DRTs[5]->m_SRVHandle);
	
	CVoxelizationShader* voxelizationShader = reinterpret_cast<CVoxelizationShader*>(CAssetManager::GetInstance()->GetShader("Voxelization"));
	gpuDescriptorHeap->AddToHandle(device, srvHandle, voxelizationShader->m_VCTVoxelization3DRT->m_SRVHandle);

	DescriptorHandle cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
	gpuDescriptorHeap->AddToHandle(device, cbvHandle, voxelizationShader->m_cpuVoxelization);
	gpuDescriptorHeap->AddToHandle(device, cbvHandle, m_VCTMainCBCPUHandle);

	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
	CTransform* transform = reinterpret_cast<CTransform*>(camera->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT4 cameraPos = XMFLOAT4(transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z, 1.0f);
	
	m_VCTMainCBMappedData->CameraPos = cameraPos;
	m_VCTMainCBMappedData->UpsampleRatio = XMFLOAT2(1.f, 1.f);
	m_VCTMainCBMappedData->IndirectDiffuseStrength = 1.0f;
	m_VCTMainCBMappedData->IndirectSpecularStrength = 1.0f;
	m_VCTMainCBMappedData->MaxConeTraceDistance = 100.0f;
	m_VCTMainCBMappedData->AOFalloff = 2.0f;
	m_VCTMainCBMappedData->SamplingFactor = 0.5f;
	m_VCTMainCBMappedData->VoxelSampleOffset = 0.0f;


	commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());
	commandList->SetGraphicsRootDescriptorTable(1, srvHandle.GetGPUHandle());

	commandList->IASetVertexBuffers(0, 1, &framework->mFullscreenQuadVertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->DrawInstanced(4, 1, 0, 0);

	DX::ResourceTransition(commandList, m_VCTMainRT->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
}
