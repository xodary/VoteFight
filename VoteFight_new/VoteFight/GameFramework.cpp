#include "pch.h"
#include "GameFramework.h"

#include "TimeManager.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "SceneManager.h"

#include "Texture.h"

CGameFramework::CGameFramework() :
	m_hWnd(),
	m_title{},
	m_resolution(),
	m_msaa4xEnable(),
	m_msaa4xQualityLevels(),
	m_dxgiIFactory(),
	m_d3d12Device(),
	m_d3d12CommandQueue(),
	m_d3d12CommandAllocator(),
	m_d3d12GraphicsCommandList(),
	m_dxgiSwapChain(),
	m_swapChainBufferIndex(),
	m_d3d12RenderTargetBuffers{},
	m_d3d12RtvDescriptorHeap(),
	m_rtvDescriptorIncrementSize(),
	m_d3d12DepthStencilBuffer(),
	m_d3d12DepthBuffer(),
	m_d3d12DsvDescriptorHeap(),
	m_dsvDescriptorIncrementSize(),
	m_d3d12CbvSrvUavDescriptorHeap(),
	m_d3d12Fence(),
	m_fenceValues{},
	m_fenceEvent(),
	m_d3d12RootSignature(),
	m_d3d12GameFramework(),
	m_mappedGameFramework()
	//m_RenderingResultTexture(),
	//m_PostProcessingShader(),
	//m_UILayer()
	//m_socketInfo(),
{
}

CGameFramework::~CGameFramework()
{
	WaitForGpuComplete();
	ReleaseShaderVariables();

	//CloseHandle(m_FenceEvent);

	//m_DXGISwapChain->SetFullscreenState(FALSE, nullptr);

	//if (m_SocketInfo.m_Socket)
	//{
	//	closesocket(m_SocketInfo.m_Socket);
	//	WSACleanup();
	//}

	//CSceneManager::GetInstance()->ReleaseShaderVariables();

	//if (m_UILayer)
	//{
	//	m_UILayer->ReleaseResources();
	//}
}

HWND CGameFramework::GetHwnd()
{
	return m_hWnd;
}

const XMFLOAT2& CGameFramework::GetResolution()
{
	return m_resolution;
}

ID3D12Device* CGameFramework::GetDevice()
{
	return m_d3d12Device.Get();
}

ID3D12CommandQueue* CGameFramework::GetCommandQueue()
{
	return m_d3d12CommandQueue.Get();
}

ID3D12GraphicsCommandList* CGameFramework::GetGraphicsCommandList()
{
	return m_d3d12GraphicsCommandList.Get();
}

ID3D12RootSignature* CGameFramework::GetRootSignature()
{
	return m_d3d12RootSignature.Get();
}

ID3D12DescriptorHeap* CGameFramework::GetRtvDescriptorHeap()
{
	return m_d3d12RtvDescriptorHeap.Get();
}

ID3D12DescriptorHeap* CGameFramework::GetDsvDescriptorHeap()
{
	return m_d3d12DsvDescriptorHeap.Get();
}

ID3D12DescriptorHeap* CGameFramework::GetCbvSrvUavDescriptorHeap()
{
	return m_d3d12CbvSrvUavDescriptorHeap.Get();
}

UINT CGameFramework::GetRtvDescriptorIncrementSize()
{
	return m_rtvDescriptorIncrementSize;
}

UINT CGameFramework::GetDsvDescriptorIncrementSize()
{
	return m_dsvDescriptorIncrementSize;
}

void CGameFramework::Init(HWND hWnd, const XMFLOAT2& resolution)
{
	m_hWnd = hWnd;
	m_resolution = resolution;

	ConnectServer();

	CreateDevice();
	CreateCommandQueueAndList();
	CreateSwapChain();
	CreateRootSignature();
	CreateShaderVariables();

	// Close ������ Ŀ�ǵ帮��Ʈ�� Open ���·� �����Ų��.
	DX::ThrowIfFailed(m_d3d12GraphicsCommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));

	CAssetManager::GetInstance()->Init();
	CCameraManager::GetInstance()->Init();
	CSceneManager::GetInstance()->Init();
	CInputManager::GetInstance()->Init();
	CTimeManager::GetInstance()->Init();

	// RenderTarget, DepthStencil
	CreateRtvAndDsvDescriptorHeaps();
	CreateRenderTargetViews();
	CreateDepthStencilView();

	// Constant / Shader Resource / Unoreded Access
	// ��� �ؽ�ó�� �ε��ߴٸ�, �ش� ������ŭ Descriptor Heap�� �Ҵ��Ѵ�.
	// * �� �����ӿ�ũ���� CbvSrvUav Descriptor Heap�� �ؽ�ó(SRV)���� �����Ѵ�.
	CreateCbvSrvUavDescriptorHeaps();
	CreateShaderResourceViews();

	CreateShaderVariables();

	// Ŀ�ǵ帮��Ʈ�� Close ���·� �����.
	DX::ThrowIfFailed(m_d3d12GraphicsCommandList->Close());

	// Ŀ�ǵ帮��Ʈ�� �����ϰ�, GPU�� ������ ����Ͽ� �ʱ� ������ ����ģ��.
	ID3D12CommandList* d3d12CommandLists[] = { m_d3d12GraphicsCommandList.Get() };

	m_d3d12CommandQueue->ExecuteCommandLists(_countof(d3d12CommandLists), d3d12CommandLists);
	WaitForGpuComplete();

	// Ŀ�ǵ帮��Ʈ�� ��� ����Ǿ��ٸ�, ���ҽ� ������ ����ߴ� ��� ���ε� ���۸� �����Ѵ�.
	CAssetManager::GetInstance()->ReleaseUploadBuffers();
	CSceneManager::GetInstance()->ReleaseUploadBuffers();
}

void CGameFramework::CreateDevice()
{
	UINT FlagCount = 0;

#ifdef _DEBUG
	ComPtr<ID3D12Debug> D3D12DebugController = nullptr;

	DX::ThrowIfFailed(D3D12GetDebugInterface(__uuidof(ID3D12Debug), reinterpret_cast<void**>(D3D12DebugController.GetAddressOf())));

	if (D3D12DebugController)
	{
		D3D12DebugController->EnableDebugLayer();
	}

	FlagCount |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	DX::ThrowIfFailed(CreateDXGIFactory2(FlagCount, __uuidof(IDXGIFactory4), reinterpret_cast<void**>(m_dxgiIFactory.GetAddressOf())));

	ComPtr<IDXGIAdapter1> DXGIAdapter = nullptr;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_dxgiIFactory->EnumAdapters1(i, DXGIAdapter.GetAddressOf()); ++i)
	{
		DXGI_ADAPTER_DESC1 DXGIAdapterDesc = {};
		HRESULT Result = 0;

		DXGIAdapter->GetDesc1(&DXGIAdapterDesc);

		if (DXGIAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		Result = D3D12CreateDevice(DXGIAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), reinterpret_cast<void**>(m_d3d12Device.GetAddressOf()));

		if (SUCCEEDED(Result))
		{
			break;
		}
		else
		{
			DX::ThrowIfFailed(Result);
		}
	}

	// ��� �ϵ���� ����� ���Ͽ� Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� �����Ѵ�.
	if (DXGIAdapter == nullptr)
	{
		DX::ThrowIfFailed(m_dxgiIFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), reinterpret_cast<void**>(DXGIAdapter.GetAddressOf())));
		DX::ThrowIfFailed(D3D12CreateDevice(DXGIAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), reinterpret_cast<void**>(m_d3d12Device.GetAddressOf())));
	}

	// Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� �����Ѵ�.
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS D3D12MsaaQualityLevels = {};

	D3D12MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D12MsaaQualityLevels.SampleCount = 4;

	// Msaa4x ���� ���ø�
	D3D12MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	D3D12MsaaQualityLevels.NumQualityLevels = 0;

	// ����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	DX::ThrowIfFailed(m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &D3D12MsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)));

	// ���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�.
	m_msaa4xQualityLevels = D3D12MsaaQualityLevels.NumQualityLevels;
	m_msaa4xEnable = (m_msaa4xQualityLevels > 1) ? true : false;

	// �潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�.
	DX::ThrowIfFailed(m_d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(m_d3d12Fence.GetAddressOf())));

	// �潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�.(�̺�Ʈ ��ü�� �ʱⰪ�� FALSE�̴�.)
	// �̺�Ʈ�� ����Ǹ�(Signal) �̺�Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC D3D12CommandQueueDesc = {};

	D3D12CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	D3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&D3D12CommandQueueDesc, _uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(m_d3d12CommandQueue.GetAddressOf())));
	DX::ThrowIfFailed(m_d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(m_d3d12CommandAllocator.GetAddressOf())));
	DX::ThrowIfFailed(m_d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_d3d12CommandAllocator.Get(), nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(m_d3d12GraphicsCommandList.GetAddressOf())));

	// ��� ����Ʈ�� �����Ǹ� ����(Open) �����̹Ƿ� ����(Closed) ���·� �����.
	DX::ThrowIfFailed(m_d3d12GraphicsCommandList->Close());
}

void CGameFramework::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc = {};

	DXGISwapChainDesc.BufferDesc.Width = static_cast<UINT>(m_resolution.x);
	DXGISwapChainDesc.BufferDesc.Height = static_cast<UINT>(m_resolution.y);
	DXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	DXGISwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	DXGISwapChainDesc.SampleDesc.Count = (m_msaa4xEnable) ? 4 : 1;
	DXGISwapChainDesc.SampleDesc.Quality = (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0;
	DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	DXGISwapChainDesc.BufferCount = m_swapChainBufferCount;
	DXGISwapChainDesc.OutputWindow = m_hWnd;
	DXGISwapChainDesc.Windowed = true;
	DXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	DXGISwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DX::ThrowIfFailed(m_dxgiIFactory->CreateSwapChain(m_d3d12CommandQueue.Get(), &DXGISwapChainDesc, reinterpret_cast<IDXGISwapChain**>(m_dxgiSwapChain.GetAddressOf())));

	// ����ü���� ���� �ĸ���� �ε����� �����Ѵ�.
	m_swapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	// "Alt+Enter" Ű�� ������ ��Ȱ��ȭ�Ѵ�.
	DX::ThrowIfFailed(m_dxgiIFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC D3D12DescriptorHeapDesc = {};

	D3D12DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	D3D12DescriptorHeapDesc.NumDescriptors = m_swapChainBufferCount + 1; // + 2: DepthWrite, PostProcessing
	D3D12DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	D3D12DescriptorHeapDesc.NodeMask = 0;

	// ���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�.
	DX::ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&D3D12DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(m_d3d12RtvDescriptorHeap.GetAddressOf())));

	// ���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.
	m_rtvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�.
	D3D12DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	D3D12DescriptorHeapDesc.NumDescriptors = 2;

	DX::ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&D3D12DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(m_d3d12DsvDescriptorHeap.GetAddressOf())));

	// ����-���ٽ� ������ ���� ������ ũ�⸦ �����Ѵ�.
	m_dsvDescriptorIncrementSize = m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateCbvSrvUavDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC D3D12DescriptorHeapDesc = {};

	D3D12DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	D3D12DescriptorHeapDesc.NumDescriptors = CAssetManager::GetInstance()->GetTextureCount();
	D3D12DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	D3D12DescriptorHeapDesc.NodeMask = 0;

	DX::ThrowIfFailed(m_d3d12Device->CreateDescriptorHeap(&D3D12DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(m_d3d12CbvSrvUavDescriptorHeap.GetAddressOf())));
}

void CGameFramework::CreateRenderTargetViews()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12RtvCpuDescriptorHandle(m_d3d12RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < m_swapChainBufferCount; ++i)
	{
		DX::ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), reinterpret_cast<void**>(m_d3d12RenderTargetBuffers[i].GetAddressOf())));
		m_d3d12Device->CreateRenderTargetView(m_d3d12RenderTargetBuffers[i].Get(), nullptr, D3D12RtvCpuDescriptorHandle);

		D3D12RtvCpuDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;
	}

	// DepthWrite
	CTexture* texture = CAssetManager::GetInstance()->GetTexture("DepthWrite");
	D3D12_RENDER_TARGET_VIEW_DESC d3d12RenderTargetViewDesc = {};

	d3d12RenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	d3d12RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3d12RenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3d12RenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	m_d3d12Device->CreateRenderTargetView(texture->GetTexture(), &d3d12RenderTargetViewDesc, D3D12RtvCpuDescriptorHandle);
	D3D12RtvCpuDescriptorHandle.ptr += m_rtvDescriptorIncrementSize;

	// PostProcessing
	//m_RenderingResultTexture = make_shared<CTexture>();
	//m_RenderingResultTexture->CreateTexture2D(m_d3d12Device.Get(), TEXTURE_TYPE_ALBEDO_MAP, static_cast<UINT>(m_resolution.x), static_cast<UINT>(m_resolution.y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R8G8B8A8_UNORM, { 0.0f, 0.0f, 0.0f, 1.0f } });
	//CTextureManager::GetInstance()->RegisterTexture(TEXT("RenderingResult"), m_RenderingResultTexture);

	//D3D12_RENDER_TARGET_VIEW_DESC D3D12RenderTargetViewDesc{};

	//D3D12RenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//D3D12RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//D3D12RenderTargetViewDesc.Texture2D.MipSlice = 0;
	//D3D12RenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	//m_d3d12Device->CreateRenderTargetView(m_RenderingResultTexture->GetResource(), &D3D12RenderTargetViewDesc, D3D12RtvCPUDescriptorHandle);
}

void CGameFramework::CreateDepthStencilView()
{
	CD3DX12_RESOURCE_DESC D3D12ResourceDesc(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, static_cast<UINT>(m_resolution.x), static_cast<UINT>(m_resolution.y), 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, (m_msaa4xEnable) ? static_cast<UINT>(4) : static_cast<UINT>(1), (m_msaa4xEnable) ? (m_msaa4xQualityLevels - 1) : 0, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	CD3DX12_HEAP_PROPERTIES D3D312HeapProperties(D3D12_HEAP_TYPE_DEFAULT, 1, 1);
	CD3DX12_CLEAR_VALUE D3D12ClearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0);

	// ����-���ٽ� ���۸� �����Ѵ�.
	DX::ThrowIfFailed(m_d3d12Device->CreateCommittedResource(&D3D312HeapProperties, D3D12_HEAP_FLAG_NONE, &D3D12ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &D3D12ClearValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(m_d3d12DepthStencilBuffer.GetAddressOf())));

	CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12DsvCPUDescriptorHandle(m_d3d12DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// ����-���ٽ� ���� �並 �����Ѵ�.
	m_d3d12Device->CreateDepthStencilView(m_d3d12DepthStencilBuffer.Get(), nullptr, D3D12DsvCPUDescriptorHandle);
	D3D12DsvCPUDescriptorHandle.ptr += m_dsvDescriptorIncrementSize;

	// Depth Write
	D3D12_DEPTH_STENCIL_VIEW_DESC d3d12DepthStencilViewDesc = {};

	d3d12DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3d12DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3d12DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_d3d12DepthBuffer = DX::CreateTexture2DResource(m_d3d12Device.Get(), DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT, 1, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, DXGI_FORMAT_D32_FLOAT, D3D12_CLEAR_VALUE{ DXGI_FORMAT_D32_FLOAT, {1.0f, 0.0f} });
	m_d3d12Device->CreateDepthStencilView(m_d3d12DepthBuffer.Get(), &d3d12DepthStencilViewDesc, D3D12DsvCPUDescriptorHandle);
}

void CGameFramework::CreateShaderResourceViews()
{
	CAssetManager::GetInstance()->CreateShaderResourceViews();
}

void CGameFramework::CreateRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE d3d12DescriptorRanges[static_cast<int>(TEXTURE_TYPE::COUNT)] = {};

	d3d12DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// Albedo
	d3d12DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	// Normal
	d3d12DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);	// Cube
	d3d12DescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);	// Shadow Map

	CD3DX12_ROOT_PARAMETER d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::COUNT)] = {};

	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::GameFramework)].InitAsConstantBufferView(0);					    // CB_GameFramework : register(b0)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::CAMERA)].InitAsConstantBufferView(1);						        // CB_Camera : register(b1)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::GAME_SCENE)].InitAsConstantBufferView(2);					        // CB_GameScene : register(b2)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::OBJECT)].InitAsConstants(26, 3);							        // CB_Object : register(b3)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::SPRITE)].InitAsConstants(4, 4);							        // CB_Sprite : register(b4)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::BONE_OFFSET)].InitAsConstantBufferView(5);						// CB_BoneOffset : register(b5)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::BONE_TRANSFORM)].InitAsConstantBufferView(6);					    // CB_BoneTransform : register(b6)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::ALBEDO_MAP)].InitAsDescriptorTable(1, &d3d12DescriptorRanges[0]); // albedoMap : register(t0)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::NORMAL_MAP)].InitAsDescriptorTable(1, &d3d12DescriptorRanges[1]); // normalMap : register(t1)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::CUBE_MAP)].InitAsDescriptorTable(1, &d3d12DescriptorRanges[2]);	// cubeMap : register(t2)
	d3d12RootParameters[static_cast<int>(ROOT_PARAMETER_TYPE::SHADOW_MAP)].InitAsDescriptorTable(1, &d3d12DescriptorRanges[3]); // shadowMap : register(t3)

	D3D12_ROOT_SIGNATURE_FLAGS d3d12RootSignatureFlags = { D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT }; // IA�ܰ踦 ���, ��Ʈ�� ��� �ܰ踦 ���
	CD3DX12_STATIC_SAMPLER_DESC d3d12SamplerDesc[3] = {};

	d3d12SamplerDesc[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 1, D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL);                            // samplerState : register(s0)
	d3d12SamplerDesc[1].Init(1, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.0f, 1, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL); // pcfSamplerState : register(s1)
	d3d12SamplerDesc[2].Init(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 1, D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX, D3D12_SHADER_VISIBILITY_PIXEL);						   // gssClamp : register(s2)

	CD3DX12_ROOT_SIGNATURE_DESC d3d12RootSignatureDesc = {};

	d3d12RootSignatureDesc.Init(_countof(d3d12RootParameters), d3d12RootParameters, _countof(d3d12SamplerDesc), d3d12SamplerDesc, d3d12RootSignatureFlags);

	ComPtr<ID3DBlob> d3d12SignatureBlob = nullptr, d3d12ErrorBlob = nullptr;

	DX::ThrowIfFailed(D3D12SerializeRootSignature(&d3d12RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, d3d12SignatureBlob.GetAddressOf(), d3d12ErrorBlob.GetAddressOf()));
	if (d3d12ErrorBlob) {
		char* pErrorString = static_cast<char*>(d3d12ErrorBlob->GetBufferPointer());
		std::cerr << "Direct3D Error: " << pErrorString << std::endl;
	}
	else {
		std::cerr << "Direct3D Error: Unknown error" << std::endl;
	}
	DX::ThrowIfFailed(m_d3d12Device->CreateRootSignature(0, d3d12SignatureBlob->GetBufferPointer(), d3d12SignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(m_d3d12RootSignature.GetAddressOf())));
}

void CGameFramework::CreateShaderVariables()
{
	UINT bytes = (sizeof(CB_GameFramework) + 255) & ~255;

	m_d3d12GameFramework = DX::CreateBufferResource(m_d3d12Device.Get(), m_d3d12GraphicsCommandList.Get(), nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12GameFramework->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedGameFramework)));
}

void CGameFramework::UpdateShaderVariables()
{
	m_d3d12GraphicsCommandList->SetGraphicsRootSignature(m_d3d12RootSignature.Get());
	m_d3d12GraphicsCommandList->SetDescriptorHeaps(1, m_d3d12CbvSrvUavDescriptorHeap.GetAddressOf());

	m_mappedGameFramework->m_totalTime += DT;
	m_mappedGameFramework->m_elapsedTime = DT;
	m_d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::GameFramework), m_d3d12GameFramework->GetGPUVirtualAddress());
}

void CGameFramework::ReleaseShaderVariables()
{
	if (m_mappedGameFramework != nullptr)
	{
		m_d3d12GameFramework->Unmap(0, nullptr);
	}
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL FullScreenState = false;

	DX::ThrowIfFailed(m_dxgiSwapChain->GetFullscreenState(&FullScreenState, nullptr));
	DX::ThrowIfFailed(m_dxgiSwapChain->SetFullscreenState(!FullScreenState, nullptr));

	DXGI_MODE_DESC DXGIModeDesc = {};

	DXGIModeDesc.Width = static_cast<UINT>(m_resolution.x);
	DXGIModeDesc.Height = static_cast<UINT>(m_resolution.y);
	DXGIModeDesc.RefreshRate.Numerator = 60;
	DXGIModeDesc.RefreshRate.Denominator = 1;
	DXGIModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGIModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	DXGIModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DX::ThrowIfFailed(m_dxgiSwapChain->ResizeTarget(&DXGIModeDesc));

	for (UINT i = 0; i < m_swapChainBufferCount; ++i)
	{
		if (m_d3d12RenderTargetBuffers[i])
		{
			m_d3d12RenderTargetBuffers[i]->Release();
		}
	}

	DXGI_SWAP_CHAIN_DESC DXGISwapChainDesc = {};

	DX::ThrowIfFailed(m_dxgiSwapChain->GetDesc(&DXGISwapChainDesc));
	DX::ThrowIfFailed(m_dxgiSwapChain->ResizeBuffers(m_swapChainBufferCount, static_cast<UINT>(m_resolution.x), static_cast<UINT>(m_resolution.y), DXGISwapChainDesc.BufferDesc.Format, DXGISwapChainDesc.Flags));

	m_swapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::ResetCommandAllocatorAndList()
{
	DX::ThrowIfFailed(m_d3d12CommandAllocator->Reset());
	DX::ThrowIfFailed(m_d3d12GraphicsCommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 FenceValue = ++m_fenceValues[m_swapChainBufferIndex];

	// GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�.
	DX::ThrowIfFailed(m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), FenceValue));

	// �潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
	if (m_d3d12Fence->GetCompletedValue() < FenceValue)
	{
		DX::ThrowIfFailed(m_d3d12Fence->SetEventOnCompletion(FenceValue, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	// ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������ ������۷� �Ű����� ���� Ÿ�� �ε����� �ٲ� ���̴�.
	m_swapChainBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	WaitForGpuComplete();

	//if (CSceneManager::GetInstance()->GetReservedScene())
	//{
	//	if (m_PostProcessingShader)
	//	{
	//		if (m_PostProcessingShader->GetPostProcessingType() == POST_PROCESSING_TYPE_NONE)
	//		{
	//			m_PostProcessingShader->SetPostProcessingType(POST_PROCESSING_TYPE_FADE_IN);

	//			CSceneManager::GetInstance()->ChangeToReservedScene();
	//		}
	//	}
	//}

	//CSoundManager::GetInstance()->Update();
}

void CGameFramework::PreRender()
{
	CSceneManager::GetInstance()->PreRender();
}

void CGameFramework::Render()
{
	CSceneManager::GetInstance()->Render();
}

void CGameFramework::PostRender()
{
}

void CGameFramework::PopulateCommandList()
{
	ResetCommandAllocatorAndList();
	UpdateShaderVariables();

	CSceneManager::GetInstance()->Update();
	CCameraManager::GetInstance()->Update();

	PreRender();

	DX::ResourceTransition(m_d3d12GraphicsCommandList.Get(), m_d3d12RenderTargetBuffers[m_swapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	CD3DX12_CPU_DESCRIPTOR_HANDLE  D3D12RtvCPUDescriptorHandle(m_d3d12RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE D3D12DsvCPUDescriptorHandle(m_d3d12DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12RtvCPUDescriptorHandle = D3D12RtvCPUDescriptorHandle.Offset(m_rtvDescriptorIncrementSize * m_swapChainBufferIndex);

	m_d3d12GraphicsCommandList->ClearRenderTargetView(D3D12RtvCPUDescriptorHandle, Colors::Pink, 0, nullptr);
	m_d3d12GraphicsCommandList->ClearDepthStencilView(D3D12DsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_d3d12GraphicsCommandList->OMSetRenderTargets(1, &D3D12RtvCPUDescriptorHandle, TRUE, &D3D12DsvCPUDescriptorHandle);

	Render();

	DX::ResourceTransition(m_d3d12GraphicsCommandList.Get(), m_d3d12RenderTargetBuffers[m_swapChainBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	DX::ThrowIfFailed(m_d3d12GraphicsCommandList->Close());

	ComPtr<ID3D12CommandList> D3D12CommandLists[] = { m_d3d12GraphicsCommandList };

	m_d3d12CommandQueue->ExecuteCommandLists(_countof(D3D12CommandLists), D3D12CommandLists->GetAddressOf());

	WaitForGpuComplete();
}

void CGameFramework::AdvanceFrame()
{
	CTimeManager::GetInstance()->Update();
	CInputManager::GetInstance()->Update();

	PopulateCommandList();
	DX::ThrowIfFailed(m_dxgiSwapChain->Present(1, 0));
	MoveToNextFrame();
}


// Server

void CGameFramework::ConnectServer()
{
	WSADATA Wsa{};

	if (WSAStartup(MAKEWORD(2, 2), &Wsa))
	{
		cout << "������ �ʱ�ȭ���� ���߽��ϴ�." << endl;
		exit(1);
	}

	m_SocketInfo.m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_SocketInfo.m_Socket == INVALID_SOCKET)
	{
		Server::ErrorQuit("socket()");
	}

	m_SocketInfo.m_SocketAddress.sin_family = AF_INET;
	m_SocketInfo.m_SocketAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	m_SocketInfo.m_SocketAddress.sin_port = htons(SERVER_PORT);

	int ReturnValue{ connect(m_SocketInfo.m_Socket, (SOCKADDR*)&m_SocketInfo.m_SocketAddress, sizeof(m_SocketInfo.m_SocketAddress)) };

	if (ReturnValue == SOCKET_ERROR)
	{
		Server::ErrorQuit("connect()");
	}

	// �÷��̾� ���̵� �����Ѵ�.
	ReturnValue = recv(m_SocketInfo.m_Socket, (char*)&m_SocketInfo.m_ID, sizeof(UINT), MSG_WAITALL);

	if (ReturnValue == SOCKET_ERROR)
	{
		Server::ErrorDisplay("recv()");
	}
	else if (m_SocketInfo.m_ID == UINT_MAX)
	{
		MessageBox(m_hWnd, TEXT("���� ������ ��á�ų�, ������ �̹� ���۵Ǿ� ������ �� �����ϴ�."), TEXT("VOTE FIGHT"), MB_ICONSTOP | MB_OK);
		PostQuitMessage(0);
	}

	//CreateEvents();
}