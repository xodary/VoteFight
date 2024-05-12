#pragma once
#include "Shader.h"

class CVoxelizationShader : public CShader
{
	friend class CAssetManager;
	
	struct CB_VOXELIZATION
	{
		XMFLOAT4X4 WorldVoxelCube;
		XMFLOAT4X4 ViewProjection;
		XMFLOAT4X4 ShadowViewProjection;
		float	   WorldVoxelScale;
	};

	struct CB_VOXELIZATION_DEBUG
	{
		XMFLOAT4X4 WorldVoxelCube;
		XMFLOAT4X4 ViewProjection;
		float	   WorldVoxelScale;
	};

	struct CB_MODEL
	{
		XMFLOAT4X4 World;
		XMFLOAT4   DiffuseColor;
	};

private:
	CVoxelizationShader();

	ComPtr<ID3D12Resource>					   m_VoxelizationBuffer;
	CB_VOXELIZATION*						   m_VoxelizationMappedData;

	ComPtr<ID3D12Resource>					   m_VoxelizationDebufBuffer;
	CB_VOXELIZATION_DEBUG*					   m_VoxelizationDebugMappedData;
	DescriptorHandle						   m_cpuVoxelizationDebug;

	ComPtr<ID3D12Resource>					   m_ModelBuffer;
	CB_MODEL*								   m_ModelMappedData;
	DescriptorHandle						   m_cpuModel;

	Texture3D*								   mVCTVoxelizationDebugRT;

	ID3D12DescriptorHeap*					   DsvDescriptorHeap;
	ID3D12RootSignature*					   m_rootSignature[2];

	// DescriptorHeapManager*					   m_DescriptorHeapManager;
public:
	virtual ~CVoxelizationShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType(int stateNum);
	virtual ID3D12RootSignature* CreateRootSignature(int stateNum);
	virtual DXGI_FORMAT GetRTVFormat(int stateNum);
	virtual DXGI_FORMAT GetDSVFormat(int stateNum);

	Texture3D* GetVoxelTexture() { return m_VCTVoxelization3DRT; }
	void Render(int stateNum);

	Texture3D*								   m_VCTVoxelization3DRT;
	DescriptorHandle						   m_cpuVoxelization;
};

