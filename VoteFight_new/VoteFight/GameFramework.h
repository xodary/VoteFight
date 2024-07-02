#pragma once
#include "DescriptorHeap.h"
#include "Mesh.h"

class CPostProcessingShader;
class CUILayer;
class DescriptorHeapManager;
class DescriptorHandle;

struct CB_GameFramework
{
	float m_totalTime;
	float m_elapsedTime;
};

class CGameFramework : public CSingleton<CGameFramework>
{
	friend class CSingleton<CGameFramework>;

public:
	// ������ ���� ��� ����
	HWND							  m_hWnd;		// ���� ������ �ڵ�
	char							  m_title[64];  // ������ Ÿ��Ʋ
	XMFLOAT2						  m_resolution; // ������ �ػ�

	// DirectX12 ���� ��� ����
	bool							  m_msaa4xEnable;
	UINT							  m_msaa4xQualityLevels;

	ComPtr<IDXGIFactory4>			  m_dxgiIFactory;
	ComPtr<ID3D12Device>			  m_d3d12Device;

	ComPtr<ID3D12CommandQueue>		  m_d3d12CommandQueue;
	ComPtr<ID3D12CommandAllocator>	  m_d3d12CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_d3d12GraphicsCommandList;

	ComPtr<IDXGISwapChain3>			  m_dxgiSwapChain;
	static const UINT				  m_swapChainBufferCount = 2;

	ComPtr<ID3D12DescriptorHeap>	  m_d3d12RtvDescriptorHeap;
	UINT							  m_rtvDescriptorIncrementSize;

	ComPtr<ID3D12Resource>			  m_d3d12DepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap>	  m_d3d12DsvDescriptorHeap;
	UINT							  m_dsvDescriptorIncrementSize;

	//ComPtr<ID3D12DescriptorHeap>	  m_d3d12CbvSrvUavDescriptorHeap;

	// Descriptor Heap Manager
	DescriptorHeapManager*			  m_DescriptorHeapManager;

	ComPtr<ID3D12Fence>				  m_d3d12Fence;
	UINT64							  m_fenceValues[m_swapChainBufferCount];
	HANDLE							  m_fenceEvent;

	ComPtr<ID3D12RootSignature>		  m_d3d12RootSignature;

	ComPtr<ID3D12Resource>			  m_d3d12GameFramework;
	CB_GameFramework*				  m_mappedGameFramework;

	HANDLE							  m_ReceiveEvent{};
	HANDLE							  m_RenderingEvent{};

private:
	CGameFramework();
	~CGameFramework();

	void CreateDevice();
	void CreateCommandQueueAndList();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateShaderResourceViews();
	void CreateRootSignature();

	void CreateShaderVariables();
	void UpdateShaderVariables();
	void ReleaseShaderVariables();

	void ChangeSwapChainState();

	void ResetCommandAllocatorAndList();
	void WaitForGpuComplete();
	void MoveToNextFrame();

	void PreRender();
	void Render();
	void PostRender();

	void PopulateCommandList();

public:
	HWND GetHwnd();
	const XMFLOAT2& GetResolution();

	ID3D12Device* GetDevice();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList* GetGraphicsCommandList();
	ID3D12RootSignature* GetRootSignature();

	ID3D12DescriptorHeap* GetRtvDescriptorHeap();
	ID3D12DescriptorHeap* GetDsvDescriptorHeap();

	DescriptorHeapManager* GetDescriptorHeapManager() { return m_DescriptorHeapManager; }
	
	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_d3d12DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }

	UINT GetRtvDescriptorIncrementSize();
	UINT GetDsvDescriptorIncrementSize();

	void Init(HWND hWnd, const XMFLOAT2& resolution);

	void AdvanceFrame();

	unsigned int my_id;

public:

	void CreateFullscreenQuadBuffers();

	ComPtr<ID3D12Resource>              mFullscreenQuadVertexBuffer;
	ComPtr<ID3D12Resource>              mFullscreenQuadVertexBufferUpload;

	D3D12_VERTEX_BUFFER_VIEW            mFullscreenQuadVertexBufferView;
	D3D12_VERTEX_BUFFER_VIEW& GetFullscreenQuadBufferView() { return mFullscreenQuadVertexBufferView; }


	DescriptorHandle				  SRVHandle[2];
	UINT							  m_swapChainBufferIndex;
	ComPtr<ID3D12Resource>			  m_d3d12RenderTargetBuffers[m_swapChainBufferCount];

	ID3D12DescriptorHeap*			  m_GUISrvDescHeap;

	std::vector<CTextMesh::FontType>  m_FontData;

};
