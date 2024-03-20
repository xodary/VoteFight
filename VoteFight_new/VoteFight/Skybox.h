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
	CSkyBox(float fWidth = 200.0f, float fHeight = 200.0f, float fDepth = 200.0f);
	virtual ~CSkyBox();
	virtual void Render(CCamera* camera);
};
