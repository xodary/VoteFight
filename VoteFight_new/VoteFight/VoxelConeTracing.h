#pragma once
#include "Object.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"

#define VCT_SCENE_VOLUME_SIZE 256
#define VCT_MIPS 6

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