#pragma once
#include "Shader.h"

struct CB_VOXELIZATION
{
	XMFLOAT4X4 WorldVoxelCube;
	XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 ShadowViewProjection;
	float	   WorldVoxelScale;
};

struct CB_MODEL
{
	XMFLOAT4X4 World;
	XMFLOAT4   DiffuseColor;
};

class CVoxelizationShader : public CShader
{
	friend class CAssetManager;

	class VCTTexture : public CTexture {
		int					rootSignature;
	public:
		void Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, int rootSignature, UINT16 depth=-1, UINT16 mips=1);
	};

private:
	CVoxelizationShader();

	ComPtr<ID3D12Resource>					   m_VoxelizationBuffer;
	CB_VOXELIZATION*						   m_VoxelizationMappedData;
	DescriptorHandle						   m_cpuVoxelization;
	ComPtr<ID3D12Resource>					   m_ModelBuffer;
	CB_MODEL*								   m_ModelMappedData;
	DescriptorHandle						   m_cpuModel;

	VCTTexture*								   m_VCTVoxelization3DRT;

	ID3D12DescriptorHeap*					   RtvDescriptorHeap;
	ID3D12DescriptorHeap*					   DsvDescriptorHeap;
	ID3D12RootSignature*					   rootSignature;

	DescriptorHeapManager*					   m_DescriptorHeapManager;
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
	
	void UpdateShaderVariables();
	void Render();
};

