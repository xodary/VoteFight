#pragma once
#include "Component.h"

class CCollider : public CComponent
{
public:
	BoundingBox				 m_origin;
	BoundingBox              m_boundingBox;

	BoundingBox				 m_MeshOrigin;
	BoundingBox              m_MeshboundingBox;

	ComPtr<ID3D12Resource>	 m_d3d12PositionBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12PositionBufferView;
	XMFLOAT3*				 m_mappedPositions;

	bool					 m_boxcollider;
	bool					 m_meshcollider;

	CCollider();
	virtual ~CCollider();

	void SetBoundingBox(const XMFLOAT3& center, const XMFLOAT3& extents);
	void SetMeshBoundingBox(const XMFLOAT3& center, const XMFLOAT3& extents);
	const BoundingBox& GetBoundingBox();
	const BoundingBox& GetMeshBoundingBox();

	virtual void Update();
	virtual void Render(CCamera* camera);
};
