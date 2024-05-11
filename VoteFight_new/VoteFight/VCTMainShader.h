#pragma once
#include "Shader.h"
#include "Texture.h"
#include "DescriptorHeap.h"

class CVCTMainShader : public CShader
{
	friend class CAssetManager;

	struct CB_VCT_MAIN
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

public:
	CVCTMainShader();
	virtual ~CVCTMainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual ID3D12RootSignature* CreateRootSignature(int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);
	virtual DXGI_FORMAT GetRTVFormat(int stateNum);

	void Render(int stateNum);

	CTexture*									m_VCTMainRT;
	ID3D12RootSignature*						m_rootSignature;

	ComPtr<ID3D12Resource>						m_VCTMainCBResource;
	CB_VCT_MAIN*								m_VCTMainCBMappedData;
	DescriptorHandle							m_VCTMainCBCPUHandle;

};

