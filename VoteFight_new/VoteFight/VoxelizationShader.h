#pragma once

class CVoxelizationShader : public CShader
{
	friend class CAssetManager;

	enum class VCT_TEXTURE_TYPE
	{
		THIRD_MAP,

		COUNT
	};


	class VCTTexture : public CTexture {

		int			    rootSignature;
	public:
		void Create(const UINT64& Width, UINT Height, D3D12_RESOURCE_STATES D3D12ResourceStates, D3D12_RESOURCE_FLAGS D3D12ResourceFlags, DXGI_FORMAT DxgiFormat, const D3D12_CLEAR_VALUE& D3D12ClearValue, int rootSignature, int depth=-1, int mips=1);
		void UpdateShaderVariable();

	};

private:
	CVoxelizationShader();
	VCTTexture*								   m_VCTVoxelization3DRT;

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

	void Render();

};

