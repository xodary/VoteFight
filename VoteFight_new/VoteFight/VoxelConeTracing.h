#pragma once
#include "Object.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "Buffer.h"

#define VCT_SCENE_VOLUME_SIZE 256
#define VCT_MIPS 6

class CDepthBuffer
{
public:
	CDepthBuffer(ID3D12Device* device, DESC::DescriptorHeapManager* descriptorManager, int width, int height, DXGI_FORMAT format);
	~CDepthBuffer();

	ID3D12Resource* GetResource() { return mDepthStencilResource.Get(); }
	DXGI_FORMAT GetFormat() { return mFormat; }
	void TransitionTo(std::vector<CD3DX12_RESOURCE_BARRIER>& barriers, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter);

	DESC::DescriptorHandle GetDSV()
	{
		return mDescriptorDSV;
	}

	DESC::DescriptorHandle& GetSRV()
	{
		return mDescriptorSRV;
	}

	const int GetWidth() { return mWidth; }
	const int GetHeight() { return mHeight; }

private:

	int mWidth, mHeight;
	DXGI_FORMAT mFormat;
	D3D12_RESOURCE_STATES mCurrentResourceState;

	DESC::DescriptorHandle mDescriptorDSV;
	DESC::DescriptorHandle mDescriptorSRV;
	ComPtr<ID3D12Resource> mDepthStencilResource;
};

class CRenderTarget
{
public:
	CRenderTarget(int width, int height, DXGI_FORMAT aFormat, D3D12_RESOURCE_FLAGS flags, LPCWSTR name, int depth = -1, int mips = 1, D3D12_RESOURCE_STATES defaultState = D3D12_RESOURCE_STATE_RENDER_TARGET);
	~CRenderTarget();

	ID3D12Resource* GetResource() { return mRenderTarget.Get(); }

	int GetWidth() { return mWidth; }
	int GetHeight() { return mHeight; }
	int GetDepth() { return mDepth; }
	void TransitionTo(std::vector<CD3DX12_RESOURCE_BARRIER>& barriers, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter);
	D3D12_RESOURCE_STATES GetCurrentState() { return mCurrentResourceState; }

	DESC::DescriptorHandle& GetRTV(int mip = 0)
	{
		return mDescriptorRTVMipsHandles[mip];
	}

	DESC::DescriptorHandle& GetSRV()
	{
		return mDescriptorSRV;
	}

	DESC::DescriptorHandle& GetUAV(int mip = 0)
	{
		return mDescriptorUAVMipsHandles[mip];
	}

private:

	int mWidth, mHeight, mDepth;
	DXGI_FORMAT mFormat;
	D3D12_RESOURCE_STATES mCurrentResourceState;

	//DXRS::DescriptorHandle mDescriptorUAV;
	DESC::DescriptorHandle mDescriptorSRV;
	//DXRS::DescriptorHandle mDescriptorRTV;
	ComPtr<ID3D12Resource> mRenderTarget;

	std::vector<DESC::DescriptorHandle> mDescriptorUAVMipsHandles;
	std::vector<DESC::DescriptorHandle> mDescriptorRTVMipsHandles;
};

class VCT : public CSingleton<VCT>
{
	friend class CSingleton<VCT>;

	enum RenderQueue {
		GRAPHICS_QUEUE,
		COMPUTE_QUEUE
	};

public:
	VCT() {}
	~VCT() {}

	void InitVoxelConeTracing();
	void RenderObject(std::unique_ptr<CObject>& aModel, std::function<void(std::unique_ptr<CObject>&)> aCallback);
	void RenderVoxelConeTracing();

	std::vector<CD3DX12_RESOURCE_BARRIER> mBarriers;
	std::vector<std::unique_ptr<CObject>> mRenderableObjects;

	CGameFramework* framework;

		// Voxel Cone Tracing
	RootSignature mVCTVoxelizationRS;
	RootSignature mVCTMainRS;
	RootSignature mVCTMainRS_Compute;
	RootSignature mVCTMainUpsampleAndBlurRS;
	RootSignature mVCTAnisoMipmappingPrepareRS;
	RootSignature mVCTAnisoMipmappingMainRS;
	GraphicsPSO mVCTVoxelizationPSO;
	GraphicsPSO mVCTMainPSO;
	ComputePSO mVCTMainPSO_Compute;
	ComputePSO mVCTAnisoMipmappingPreparePSO;
	ComputePSO mVCTAnisoMipmappingMainPSO;
	ComputePSO mVCTMainUpsampleAndBlurPSO;
	RootSignature mVCTVoxelizationDebugRS;
	GraphicsPSO mVCTVoxelizationDebugPSO;
	CRenderTarget* mVCTVoxelization3DRT = nullptr;
	CRenderTarget* mVCTVoxelization3DRT_CopyForAsync = nullptr;
	CRenderTarget* mVCTVoxelizationDebugRT = nullptr;
	CRenderTarget* mVCTMainRT = nullptr;
	CRenderTarget* mVCTMainUpsampleAndBlurRT = nullptr;
	CRenderTarget* mVCTAnisoMipmappinPrepare3DRTs[6] = { nullptr };
	CRenderTarget* mVCTAnisoMipmappinMain3DRTs[6] = { nullptr };
	__declspec(align(16)) struct VCTVoxelizationCBData
	{
		XMMATRIX WorldVoxelCube;
		XMMATRIX ViewProjection;
		XMMATRIX ShadowViewProjection;
		float WorldVoxelScale;
	};
	__declspec(align(16)) struct VCTAnisoMipmappingCBData
	{
		int MipDimension;
		int MipLevel;
	};
	__declspec(align(16)) struct VCTMainCBData
	{
		XMFLOAT4 CameraPos;
		XMFLOAT2 UpsampleRatio;
		float IndirectDiffuseStrength;
		float IndirectSpecularStrength;
		float MaxConeTraceDistance;
		float AOFalloff;
		float SamplingFactor;
		float VoxelSampleOffset;
	};
	CBuffer* mVCTVoxelizationCB = nullptr;
	CBuffer* mVCTAnisoMipmappingCB = nullptr;
	CBuffer* mVCTMainCB = nullptr;
	std::vector<CBuffer*> mVCTAnisoMipmappingMainCB;
	CDepthBuffer* mShadowDepth = nullptr;
	bool mVCTRenderDebug = false;
	float mWorldVoxelScale = VCT_SCENE_VOLUME_SIZE * 0.5f;
	float mVCTIndirectDiffuseStrength = 1.0f;
	float mVCTIndirectSpecularStrength = 1.0f;
	float mVCTMaxConeTraceDistance = 100.0f;
	float mVCTAoFalloff = 2.0f;
	float mVCTSamplingFactor = 0.5f;
	float mVCTVoxelSampleOffset = 0.0f;
	float mVCTRTRatio = 0.5f; // from MAX_SCREEN_WIDTH/HEIGHT
	bool mVCTUseMainCompute = true;
	bool mVCTMainRTUseUpsampleAndBlur = true;
	float mVCTGIPower = 1.0f;

	D3D12_RASTERIZER_DESC mRasterizerState;
	D3D12_RASTERIZER_DESC mRasterizerStateNoCullNoDepth;
	D3D12_BLEND_DESC mBlendState;
	D3D12_DEPTH_STENCIL_DESC mDepthStateDisabled;
	D3D12_DEPTH_STENCIL_DESC mDepthStateRW;

};