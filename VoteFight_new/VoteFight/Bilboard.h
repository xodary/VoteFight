#pragma once
#include "Object.h"

class CBilboard : public CObject
{
protected:
	int						 m_maxVertexCount;
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;

public:
	CBilboard(float fWidth = 5, float fHeight = 5);
	virtual ~CBilboard();
	virtual void Render(CCamera* camera);
};
