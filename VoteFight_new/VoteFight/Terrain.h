#pragma once
#include "Object.h"


class CTerrain : public CObject
{
private:
	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

	BYTE* m_pHeightMapPixels;

	int m_nSubMeshes;
	int* m_pnSubSetIndices;
	UINT** m_ppnSubSetIndices;
protected:
	int						 m_maxVertexCount;
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;


	ComPtr<ID3D12Resource>	        m_d3d12ColorBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12ColorUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12ColorBufferView;


	ComPtr<ID3D12Resource>	        m_d3d12TextureCoord0Buffer;
	ComPtr<ID3D12Resource>	        m_d3d12TextureCoord0UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TextureCoord0BufferView;

	ComPtr<ID3D12Resource>	        m_d3d12TextureCoord1Buffer;
	ComPtr<ID3D12Resource>	        m_d3d12TextureCoord1UploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TextureCoord1BufferView;

	vector<ComPtr<ID3D12Resource>>  m_d3d12SubSetIndexBuffers;
	vector<ComPtr<ID3D12Resource>>  m_d3d12SetIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW>	m_d3d12SubSetIndexBufferViews;

	vector<ComPtr<ID3D12Resource>>  m_d3d12IndexBuffers;
	vector<ComPtr<ID3D12Resource>>  m_d3d12IndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW>	m_d3d12IndexBufferViews;

public:
	CTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CTerrain();

	void LoadHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	void MakeHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext);
	float GetHeight(float fx, float fz , bool bReverseQuad);


	virtual void Render(CCamera* camera);
};
