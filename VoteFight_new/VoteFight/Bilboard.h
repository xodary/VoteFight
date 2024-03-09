#pragma once
#include "Object.h"

struct QuadInfo
{
	XMFLOAT3 m_position;
	XMFLOAT2 m_size;

	XMUINT2  m_spriteSize;
	float	 m_spriteIndex;

	XMFLOAT4 m_color;
};

class CBilboard abstract : public CObject
{
protected:
	int						 m_maxVertexCount;
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;
	QuadInfo*				 m_mappedQuadInfo;

public:
	CBilboard(); 
	virtual ~CBilboard();

	virtual void ReleaseUploadBuffers();

	virtual void Render(CCamera* camera);
};

//=========================================================================================================================

class CSkyBox : public CBilboard
{
public:
	CSkyBox();
	virtual ~CSkyBox();

	virtual void Render(CCamera* camera);
};