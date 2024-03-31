#pragma once
#include "Object.h"


class CHeightMapImage
{
private:
	BYTE* m_pHeightMapPixels;

	int								m_nWidth;
	int								m_nLength;
	XMFLOAT3						m_xmf3Scale;

public:
	CHeightMapImage() {};
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	virtual ~CHeightMapImage();

	float GetHeight(float x, float z, bool bReverseQuad = false);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CTerrain : public CObject
{
private:
	CHeightMapImage* m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

	UINT						m_indices;
protected:
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;

	ComPtr<ID3D12Resource>	        m_d3d12TextureCoordBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12TextureCoordUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TextureCoordBufferView;

	ComPtr<ID3D12Resource>  m_d3d12IndexBuffer;
	ComPtr<ID3D12Resource>  m_d3d12IndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW	m_d3d12IndexBufferView;

public:
	CTerrain(int nWidth = 100.f, int nLength = 100.f);
	virtual ~CTerrain();

	void MakeHeightMapGridMesh( int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, void* pContext);

	float OnGetHeight(int x, int z, void* pContext);
	virtual void Render(CCamera* camera);

	XMFLOAT4 Add(const XMFLOAT4&, const XMFLOAT4&);
};

