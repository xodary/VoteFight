#include "pch.h"
#include "MainShader.h"

CMainShader::CMainShader()
{
}

CMainShader::~CMainShader()
{
}

D3D12_INPUT_LAYOUT_DESC CMainShader::CreateInputLayout(int stateNum)
{
	UINT inputElementCount = 0;
	D3D12_INPUT_ELEMENT_DESC* d3d12InputElementDescs = nullptr;

	inputElementCount = 2;
	d3d12InputElementDescs = new D3D12_INPUT_ELEMENT_DESC[inputElementCount];
	d3d12InputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3d12InputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC D3D12InputLayoutDesc = { d3d12InputElementDescs, inputElementCount };

	return D3D12InputLayoutDesc;

}

D3D12_SHADER_BYTECODE CMainShader::CreateVertexShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("main.hlsl", "VS_Main", "vs_5_1", d3d12ShaderBlob);
}

D3D12_SHADER_BYTECODE CMainShader::CreatePixelShader(ID3DBlob* d3d12ShaderBlob, int stateNum)
{
	return Compile("main.hlsl", "PS_Main", "ps_5_1", d3d12ShaderBlob);
}

D3D12_RASTERIZER_DESC CMainShader::CreateRasterizerState(int stateNum)
{
	D3D12_RASTERIZER_DESC rasterizerState = {};

	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerState.DepthClipEnable = TRUE;
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	rasterizerState.ForcedSampleCount = 0;
	rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerState;
}

D3D12_BLEND_DESC CMainShader::CreateBlendState(int stateNum)
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

D3D12_DEPTH_STENCIL_DESC CMainShader::CreateDepthStencilState(int stateNum)
{
	D3D12_DEPTH_STENCIL_DESC depthStateDisabled = {};
	D3D12_DEPTH_STENCIL_DESC depthStateRW = {};

	depthStateRW.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStateRW.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStateRW.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	depthStateDisabled.DepthEnable = FALSE;
	depthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStateDisabled.StencilEnable = FALSE;
	depthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	depthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStateDisabled.BackFace = depthStateRW.FrontFace;

	return depthStateDisabled;
}

DXGI_FORMAT CMainShader::GetRTVFormat(int stateNum)
{
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

void CMainShader::Render(int stateNum)
{
}
