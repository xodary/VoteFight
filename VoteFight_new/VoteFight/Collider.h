#pragma once
#include "Component.h"

class CCollider : public CComponent
{
private:
	BoundingBox				 m_origin;
	BoundingBox              m_boundingBox;

	ComPtr<ID3D12Resource>	 m_d3d12PositionBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12PositionBufferView;
	XMFLOAT3*				 m_mappedPositions;

public:
	CCollider();
	virtual ~CCollider();

	void SetBoundingBox(const XMFLOAT3& center, const XMFLOAT3& extents);
	const BoundingBox& GetBoundingBox();

	virtual void Update();
	virtual void Render(CCamera* camera);
};
