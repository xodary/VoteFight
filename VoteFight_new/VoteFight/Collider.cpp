#include "pch.h"
#include "Collider.h"

#include "GameFramework.h"

#include "AssetManager.h"

#include "Object.h"

#include "Mesh.h"
#include "Shader.h"

#include "Transform.h"

CCollider::CCollider() :
    m_origin(),
    m_boundingBox(),
    m_d3d12PositionBuffer(),
    m_d3d12PositionBufferView()
{
}

CCollider::~CCollider()
{
}

void CCollider::SetBoundingBox(const XMFLOAT3& center, const XMFLOAT3& extents)
{
	m_origin.Center = center;
	m_origin.Extents = extents;

	XMFLOAT3 xmf3Corners[8];
	m_origin.GetCorners(xmf3Corners);	// 바운딩 박스의 꼭짓점 정보를 가져옴.

	int i = 0;

	const UINT vertexCount = 24;
	XMFLOAT3 positions[vertexCount];

	// 꼭짓점 정보를 이용해 LineList의 형식으로 육면체의 변을 그려준다.
	positions[i++] = xmf3Corners[0];
	positions[i++] = xmf3Corners[1];

	positions[i++] = xmf3Corners[1];
	positions[i++] = xmf3Corners[2];

	positions[i++] = xmf3Corners[2];
	positions[i++] = xmf3Corners[3];

	positions[i++] = xmf3Corners[3];
	positions[i++] = xmf3Corners[0];

	positions[i++] = xmf3Corners[4];
	positions[i++] = xmf3Corners[5];

	positions[i++] = xmf3Corners[5];
	positions[i++] = xmf3Corners[6];

	positions[i++] = xmf3Corners[6];
	positions[i++] = xmf3Corners[7];

	positions[i++] = xmf3Corners[7];
	positions[i++] = xmf3Corners[4];

	positions[i++] = xmf3Corners[0];
	positions[i++] = xmf3Corners[4];

	positions[i++] = xmf3Corners[1];
	positions[i++] = xmf3Corners[5];

	positions[i++] = xmf3Corners[2];
	positions[i++] = xmf3Corners[6];

	positions[i++] = xmf3Corners[3];
	positions[i++] = xmf3Corners[7];

	if (m_mappedPositions == nullptr)
	{
		ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
		ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

		m_d3d12PositionBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, positions, sizeof(XMFLOAT3) * vertexCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
		DX::ThrowIfFailed(m_d3d12PositionBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedPositions)));
		m_d3d12PositionBufferView.BufferLocation = m_d3d12PositionBuffer->GetGPUVirtualAddress();
		m_d3d12PositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_d3d12PositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * vertexCount;
	}
	else
	{
		for (int i = 0; i < vertexCount; ++i)
		{
			m_mappedPositions[i] = positions[i];
		}
	}
}

const BoundingBox& CCollider::GetBoundingBox()
{
    return m_boundingBox;
}

void CCollider::Update()
{
    if (!m_isEnabled)
    {
        return;
    }

    CTransform* transform = static_cast<CTransform*>(m_owner->GetComponent(COMPONENT_TYPE::TRANSFORM));

    m_origin.Transform(m_boundingBox, XMLoadFloat4x4(&transform->GetWorldMatrix()));
}

void CCollider::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12PositionBufferView };
	CAssetManager::GetInstance()->GetShader("WireFrame")->SetPipelineState(0);

    d3d12GraphicsCommandList->IASetVertexBuffers(0, _countof(vertexBufferViews), vertexBufferViews);
    d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    d3d12GraphicsCommandList->DrawInstanced(24, 1, 0, 0);
}
