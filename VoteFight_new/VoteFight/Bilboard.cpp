#include "pch.h"
#include "Bilboard.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"

CBilboard::CBilboard(float fWidth, float fHeight)
{
	SetActive(true);

	m_maxVertexCount = m_vertexCount = 6;

	XMFLOAT3* m_pxmf3Positions = new XMFLOAT3[m_maxVertexCount];

	float fx = fWidth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);

	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	m_d3d12VertexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_maxVertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12VertexUploadBuffer.GetAddressOf());
	m_d3d12VertexBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12VertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3d12VertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_maxVertexCount;

	string strBliboard = "Bilboard";
	CMaterial* material = CAssetManager::GetInstance()->CreateMaterial(strBliboard);
	CTexture* texture = CAssetManager::GetInstance()->CreateTexture(strBliboard, "SkyBox_0", TEXTURE_TYPE::ALBEDO_MAP);
	CShader* shader = CAssetManager::GetInstance()->GetShader(strBliboard);

	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(0);
	AddMaterial(material);
}

CBilboard::~CBilboard()
{
}

void CBilboard::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 1, vertexBufferViews);

	CMaterial* material = GetMaterials()[0];

	material->SetPipelineState(RENDER_TYPE::STANDARD);
	XMFLOAT4X4 xmf4x4world = Matrix4x4::Identity();
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16,
		&xmf4x4world, 0);
	material->UpdateShaderVariables();

	d3d12GraphicsCommandList->DrawInstanced(m_maxVertexCount, 1, 0, 0);
}