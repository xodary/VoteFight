#pragma once
#include "Object.h"
#define TERRAIN_GROUND 0
#define TERRAIN_SEA 1

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
	int							m_type;

	UINT						m_indices;
protected:
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;

	ComPtr<ID3D12Resource>	        m_d3d12TextureCoordBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12TextureCoordUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TextureCoordBufferView;

	ComPtr<ID3D12Resource>	        m_d3d12NormalBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12NormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12NormalBufferView;

	ComPtr<ID3D12Resource>  m_d3d12IndexBuffer;
	ComPtr<ID3D12Resource>  m_d3d12IndexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW	m_d3d12IndexBufferView;

public:
	CTerrain(int nWidth = 257.f, int nLength = 257.f, int type = TERRAIN_GROUND);
	virtual ~CTerrain();

	void MakeHeightMapGridMesh( int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, void* pContext);

	float GetWidth() { return m_nWidth;};
	float GetLength(){ return m_nLength; };
	float OnGetHeight(int x, int z);
	void CreateNormalDate(const UINT* pnSubSetIndices, const XMFLOAT3* vertices, vector<XMFLOAT3>& new_NorVecs);
	
	virtual void PreRender(CCamera* camera);
	virtual void Render(CCamera* camera);

	XMFLOAT4 Add(const XMFLOAT4&, const XMFLOAT4&);
};

