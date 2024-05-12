#pragma once
#include "Asset.h"

class CShader : public CAsset
{
	friend class CAssetManager;

protected:
	vector<ComPtr<ID3D12PipelineState>> m_d3d12PipelineStates;

protected:
	CShader();

	D3D12_SHADER_BYTECODE Compile(const string& fileName, const string& shaderName, const string& shaderVersion, ID3DBlob* d3d12CodeBlob);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout(int stateNum);
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState(int stateNum);
	virtual D3D12_BLEND_DESC CreateBlendState(int stateNum);
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(int stateNum);
	virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutputState(int stateNum);

	virtual ID3D12RootSignature* CreateRootSignature(int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob* d3d12ShaderBlob, int stateNum);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType(int stateNum);
	virtual DXGI_FORMAT GetRTVFormat(int stateNum);
	virtual DXGI_FORMAT GetDSVFormat(int stateNum);

	virtual void CreatePipelineState(int stateNum);

public:
	virtual ~CShader();

	void CreatePipelineStates(int stateCount);
	void SetPipelineState(int stateNum);
};

//=========================================================================================================================

class CComputeShader : public CShader
{
	friend class CAssetManager;

private:
	XMUINT3 m_threadGroup;

protected:
	CComputeShader();

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob* d3d12ShaderBlob, int stateNum);

public:
	virtual ~CComputeShader();

	void SetThreadGroup(const XMUINT3& threadGroup);
	const XMUINT3& GetThreadGroup();

	virtual void CreatePipelineState(int stateNum);
	virtual ID3D12RootSignature* CreateComputeRootSignature(int stateNum);
	
	void Dispatch(int stateNum);
};
