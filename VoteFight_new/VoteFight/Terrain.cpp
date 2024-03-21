#include "pch.h"
#include "Terrain.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"

CTerrain::CTerrain()
{
}

CTerrain::~CTerrain()
{
}


void CTerrain::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 1, vertexBufferViews);

	CMaterial* material = GetMaterials()[0];

	material->SetPipelineState(RENDER_TYPE::STANDARD);
	XMFLOAT4X4 xmf4x4world = Matrix4x4::Identity();
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &xmf4x4world, 0);
	material->UpdateShaderVariables();

	d3d12GraphicsCommandList->DrawInstanced(m_maxVertexCount, 1, 0, 0);

	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}
