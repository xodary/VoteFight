#pragma once
#include "Asset.h"

class CMesh : public CAsset
{
	friend class CAssetManager;

protected:
	D3D12_PRIMITIVE_TOPOLOGY        m_d3d12PrimitiveTopology;
							        
	vector<XMFLOAT3>				m_positions;
	vector<vector<UINT>>			m_indices;
							        
	ComPtr<ID3D12Resource>	        m_d3d12PositionBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12PositionUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12PositionBufferView;
							        
	ComPtr<ID3D12Resource>	        m_d3d12NormalBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12NormalUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12NormalBufferView;
							        
	ComPtr<ID3D12Resource>	        m_d3d12TangentBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12TangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TangentBufferView;
							        
	ComPtr<ID3D12Resource>	        m_d3d12BiTangentBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12BiTangentUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12BiTangentBufferView;

	ComPtr<ID3D12Resource>	        m_d3d12TexCoordBuffer;
	ComPtr<ID3D12Resource>	        m_d3d12TexCoordUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW        m_d3d12TexCoordBufferView;
	
	vector<ComPtr<ID3D12Resource>>  m_d3d12IndexBuffers;
	vector<ComPtr<ID3D12Resource>>  m_d3d12IndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW>	m_d3d12IndexBufferViews;
							 
protected:
	CMesh();
	CMesh(const CMesh& rhs);

public:
	virtual ~CMesh();

	void Load(ifstream& in);

	virtual void ReleaseUploadBuffers();

	bool CheckRayIntersection(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, const XMMATRIX& worldMatrix, float& hitDistance);

	virtual void Render(int subSetIndex);
};

class CRectMesh : public CMesh
{
private:
	int m_vertexCount = 6;

public:
	CRectMesh();
	~CRectMesh();
	virtual void Render();
};