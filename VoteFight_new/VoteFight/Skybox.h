#pragma once
#include "Object.h"

class CSkyBox : public CObject
{
protected:
	int						 m_maxVertexCount;
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;

public:
	CSkyBox(float fWidth = 1000.0f, float center = 0.0f);
	virtual ~CSkyBox();
	virtual void Render(CCamera* camera);
};
