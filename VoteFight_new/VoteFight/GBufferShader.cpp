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
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();

	m_GBuffer[0] = new CTexture();
	m_GBuffer[1] = new CTexture();
	m_GBuffer[2] = new CTexture();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};

	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.Format = format;
	m_GBuffer[0]->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::ALBEDO_MAP);
	m_GBuffer[0]->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(m_GBuffer[0]->GetTexture(), &rtvDesc, m_GBuffer[0]->m_RTVHandle.GetCPUHandle());
	m_GBuffer[0]->m_SRVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_GBuffer[0]->GetTexture(), nullptr, m_GBuffer[0]->m_SRVHandle.GetCPUHandle());

	format = DXGI_FORMAT_R16G16B16A16_SNORM;
	rtvDesc.Format = format;
	m_GBuffer[1]->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_GBuffer[1]->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::ALBEDO_MAP);
	device->CreateRenderTargetView(m_GBuffer[1]->GetTexture(), &rtvDesc, m_GBuffer[1]->m_RTVHandle.GetCPUHandle());
	m_GBuffer[1]->m_SRVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_GBuffer[1]->GetTexture(), nullptr, m_GBuffer[1]->m_SRVHandle.GetCPUHandle());
	
	format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.Format = format;
	m_GBuffer[2]->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_GBuffer[2]->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::ALBEDO_MAP);
	device->CreateRenderTargetView(m_GBuffer[2]->GetTexture(), &rtvDesc, m_GBuffer[2]->m_RTVHandle.GetCPUHandle());
	m_GBuffer[2]->m_SRVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_GBuffer[2]->GetTexture(), nullptr, m_GBuffer[2]->m_SRVHandle.GetCPUHandle());

	//Create Pipeline State Object
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	UINT bytes = (sizeof(CB_GBUFFER) + 255) & ~255;
	m_GBufferCBResource = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_GBufferCBResource->Map(0, nullptr, reinterpret_cast<void**>(&m_GBufferCBMappedData)));
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.SizeInBytes = bytes;
	cbvDesc.BufferLocation = m_GBufferCBResource->GetGPUVirtualAddress();
	m_GBufferCBCPUHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateConstantBufferView(&cbvDesc, m_GBufferCBCPUHandle.GetCPUHandle());

	bytes = (sizeof(CB_MODEL) + 255) & ~255;
	m_ModelBuffer = DX::CreateBufferResource(device, commandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_ModelBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_ModelMappedData)));
	cbvDesc.SizeInBytes = bytes;
	cbvDesc.BufferLocation = m_ModelBuffer->GetGPUVirtualAddress();
	m_cpuModel = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateConstantBufferView(&cbvDesc, m_cpuModel.GetCPUHandle());

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

void CGBufferShader::Render(int stateNum)
{
	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();
	GPUDescriptorHeap* gpuDescriptorHeap = descriptorHeapManager->GetGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);
	CD3DX12_RECT rect = CD3DX12_RECT(0.0f, 0.0f, framework->GetResolution().x, framework->GetResolution().y);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &rect);

	SetPipelineState(stateNum);
	commandList->SetGraphicsRootSignature(m_rootSignature);

	DX::ResourceTransition(commandList, m_GBuffer[0]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DX::ResourceTransition(commandList, m_GBuffer[1]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	DX::ResourceTransition(commandList, m_GBuffer[2]->GetTexture(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
	{
		m_GBuffer[0]->m_RTVHandle.GetCPUHandle(),
		m_GBuffer[1]->m_RTVHandle.GetCPUHandle(),
		m_GBuffer[2]->m_RTVHandle.GetCPUHandle()
	};

	float clearColorBlack[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->OMSetRenderTargets(_countof(rtvHandles), rtvHandles, FALSE, &framework->GetDepthStencilView());
	commandList->ClearRenderTargetView(rtvHandles[0], clearColorBlack, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[1], clearColorBlack, 0, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[2], clearColorBlack, 0, nullptr);

	CObject* object = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::STRUCTURE)[0];
	CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMStoreFloat4x4(&m_ModelMappedData->World, XMLoadFloat4x4(&transform->GetWorldMatrix()));
	XMFLOAT4 f4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	memcpy(&m_ModelMappedData->DiffuseColor, &f4, sizeof(XMFLOAT4));
	
	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();
	transform = reinterpret_cast<CTransform*>(camera->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT4 cameraPos = XMFLOAT4(transform->GetPosition().x, transform->GetPosition().y, transform->GetPosition().z, 1.0f);
	m_GBufferCBMappedData->CameraPos = cameraPos;
	m_GBufferCBMappedData->ViewProjection = Matrix4x4::Multiply(camera->GetViewMatrix(), camera->GetProjectionMatrix());
	m_GBufferCBMappedData->InvViewProjection = Matrix4x4::Inverse(m_GBufferCBMappedData->ViewProjection);
	m_GBufferCBMappedData->LightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_GBufferCBMappedData->ScreenSize = XMFLOAT4(framework->GetResolution().x, framework->GetResolution().y, 1.0f / framework->GetResolution().x, 1.0f / framework->GetResolution().y);
	
	DescriptorHandle cbvHandle;
	cbvHandle = gpuDescriptorHeap->GetHandleBlock(2);
	gpuDescriptorHeap->AddToHandle(device, cbvHandle, m_GBufferCBCPUHandle);
	gpuDescriptorHeap->AddToHandle(device, cbvHandle, m_cpuModel);
	commandList->SetGraphicsRootDescriptorTable(0, cbvHandle.GetGPUHandle());

	object->GetMesh()->Render(0);

	DX::ResourceTransition(commandList, m_GBuffer[0]->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, m_GBuffer[1]->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	DX::ResourceTransition(commandList, m_GBuffer[2]->GetTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
}
