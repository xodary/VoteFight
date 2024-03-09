#include "pch.h"
#include "Mesh.h"

#include "GameFramework.h"

CMesh::CMesh() :
	m_d3d12PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
	m_positions(),
	m_indices(),
	m_d3d12PositionBuffer(),
	m_d3d12PositionUploadBuffer(),
	m_d3d12PositionBufferView(),
	m_d3d12NormalBuffer(),
	m_d3d12NormalUploadBuffer(),
	m_d3d12NormalBufferView(),
	m_d3d12TangentBuffer(),
	m_d3d12TangentUploadBuffer(),
	m_d3d12TangentBufferView(),
	m_d3d12BiTangentBuffer(),
	m_d3d12BiTangentUploadBuffer(),
	m_d3d12BiTangentBufferView(),
	m_d3d12TexCoordBuffer(),
	m_d3d12TexCoordUploadBuffer(),
	m_d3d12TexCoordBufferView(),
	m_d3d12IndexBuffers(),
	m_d3d12IndexUploadBuffers(),
	m_d3d12IndexBufferViews()
{
}

CMesh::CMesh(const CMesh& rhs) :
	m_d3d12PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
	m_positions(rhs.m_positions),
	m_indices(rhs.m_indices),
	m_d3d12PositionBuffer(rhs.m_d3d12PositionBuffer),
	m_d3d12PositionUploadBuffer(),
	m_d3d12PositionBufferView(rhs.m_d3d12PositionBufferView),
	m_d3d12NormalBuffer(rhs.m_d3d12NormalBuffer),
	m_d3d12NormalUploadBuffer(),
	m_d3d12NormalBufferView(rhs.m_d3d12NormalBufferView),
	m_d3d12TangentBuffer(rhs.m_d3d12TangentBuffer),
	m_d3d12TangentUploadBuffer(),
	m_d3d12TangentBufferView(rhs.m_d3d12TangentBufferView),
	m_d3d12BiTangentBuffer(rhs.m_d3d12BiTangentBuffer),
	m_d3d12BiTangentUploadBuffer(),
	m_d3d12BiTangentBufferView(rhs.m_d3d12BiTangentBufferView),
	m_d3d12TexCoordBuffer(rhs.m_d3d12TexCoordBuffer),
	m_d3d12TexCoordUploadBuffer(),
	m_d3d12TexCoordBufferView(rhs.m_d3d12TexCoordBufferView),
	m_d3d12IndexBuffers(rhs.m_d3d12IndexBuffers),
	m_d3d12IndexUploadBuffers(),
	m_d3d12IndexBufferViews(rhs.m_d3d12IndexBufferViews)
{
	m_name = rhs.m_name;
	m_d3d12PositionBuffer->AddRef();
	m_d3d12NormalBuffer->AddRef();
	m_d3d12TangentBuffer->AddRef();
	m_d3d12BiTangentBuffer->AddRef();
	m_d3d12TexCoordBuffer->AddRef();

	for (int i = 0; i < m_d3d12IndexBuffers.size(); ++i)
	{
		m_d3d12IndexBuffers[i]->AddRef();
	}
}

CMesh::~CMesh()
{
}

void CMesh::Load(ifstream& in)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Name>")
		{
			File::ReadStringFromFile(in, m_name);
		}
		else if (str == "<Positions>")
		{
			int VertexCount = 0;

			in.read(reinterpret_cast<char*>(&VertexCount), sizeof(int));

			if (VertexCount > 0)
			{
				m_positions.resize(VertexCount);

				in.read(reinterpret_cast<char*>(m_positions.data()), sizeof(XMFLOAT3) * VertexCount);

				m_d3d12PositionBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_positions.data(), sizeof(XMFLOAT3) * VertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12PositionUploadBuffer.GetAddressOf());
				m_d3d12PositionBufferView.BufferLocation = m_d3d12PositionBuffer->GetGPUVirtualAddress();
				m_d3d12PositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3d12PositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * VertexCount;
			}
		}
		else if (str == "<Normals>")
		{
			int VertexCount = 0;

			in.read(reinterpret_cast<char*>(&VertexCount), sizeof(int));

			if (VertexCount > 0)
			{
				vector<XMFLOAT3> Normals(VertexCount);

				in.read(reinterpret_cast<char*>(Normals.data()), sizeof(XMFLOAT3) * VertexCount);

				m_d3d12NormalBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, Normals.data(), sizeof(XMFLOAT3) * VertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12NormalUploadBuffer.GetAddressOf());
				m_d3d12NormalBufferView.BufferLocation = m_d3d12NormalBuffer->GetGPUVirtualAddress();
				m_d3d12NormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3d12NormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * VertexCount;
			}
		}
		else if (str == "<Tangents>")
		{
			int VertexCount = 0;

			in.read(reinterpret_cast<char*>(&VertexCount), sizeof(int));

			if (VertexCount > 0)
			{
				vector<XMFLOAT3> Tangents(VertexCount);

				in.read(reinterpret_cast<char*>(Tangents.data()), sizeof(XMFLOAT3) * VertexCount);

				m_d3d12TangentBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, Tangents.data(), sizeof(XMFLOAT3) * VertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12TangentUploadBuffer.GetAddressOf());
				m_d3d12TangentBufferView.BufferLocation = m_d3d12TangentBuffer->GetGPUVirtualAddress();
				m_d3d12TangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3d12TangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * VertexCount;
			}
		}
		else if (str == "<BiTangents>")
		{
			int VertexCount = 0;

			in.read(reinterpret_cast<char*>(&VertexCount), sizeof(int));

			if (VertexCount > 0)
			{
				vector<XMFLOAT3> BiTangents(VertexCount);

				in.read(reinterpret_cast<char*>(BiTangents.data()), sizeof(XMFLOAT3) * VertexCount);

				m_d3d12BiTangentBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, BiTangents.data(), sizeof(XMFLOAT3) * VertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12BiTangentUploadBuffer.GetAddressOf());
				m_d3d12BiTangentBufferView.BufferLocation = m_d3d12BiTangentBuffer->GetGPUVirtualAddress();
				m_d3d12BiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3d12BiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * VertexCount;
			}
		}
		else if (str == "<TexCoords>")
		{
			int VertexCount = 0;

			in.read(reinterpret_cast<char*>(&VertexCount), sizeof(int));

			if (VertexCount > 0)
			{
				vector<XMFLOAT2> TexCoords(VertexCount);

				in.read(reinterpret_cast<char*>(TexCoords.data()), sizeof(XMFLOAT2) * VertexCount);

				m_d3d12TexCoordBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, TexCoords.data(), sizeof(XMFLOAT2) * VertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12TexCoordUploadBuffer.GetAddressOf());
				m_d3d12TexCoordBufferView.BufferLocation = m_d3d12TexCoordBuffer->GetGPUVirtualAddress();
				m_d3d12TexCoordBufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3d12TexCoordBufferView.SizeInBytes = sizeof(XMFLOAT2) * VertexCount;
			}
		}
		else if (str == "<SubMeshes>")
		{
			int SubMeshCount = 0;

			in.read(reinterpret_cast<char*>(&SubMeshCount), sizeof(int));

			if (SubMeshCount > 0)
			{
				m_indices.resize(SubMeshCount);
				m_d3d12IndexBuffers.resize(SubMeshCount);
				m_d3d12IndexUploadBuffers.resize(SubMeshCount);
				m_d3d12IndexBufferViews.resize(SubMeshCount);

				for (int i = 0; i < SubMeshCount; ++i)
				{
					// <Indices>
					File::ReadStringFromFile(in, str);

					int IndexCount = 0;

					in.read(reinterpret_cast<char*>(&IndexCount), sizeof(int));

					if (IndexCount > 0)
					{
						m_indices[i].resize(IndexCount);

						in.read(reinterpret_cast<char*>(m_indices[i].data()), sizeof(UINT) * IndexCount);

						m_d3d12IndexBuffers[i] = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_indices[i].data(), sizeof(UINT) * IndexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_d3d12IndexUploadBuffers[i].GetAddressOf());
						m_d3d12IndexBufferViews[i].BufferLocation = m_d3d12IndexBuffers[i]->GetGPUVirtualAddress();
						m_d3d12IndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
						m_d3d12IndexBufferViews[i].SizeInBytes = sizeof(UINT) * IndexCount;
					}
				}
			}
		}
		else if (str == "</Mesh>" || str == "</SkinnedMesh>")
		{
			break;
		}
	}
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_d3d12PositionUploadBuffer.Get() != nullptr)
	{
		m_d3d12PositionUploadBuffer.Reset();
	}

	if (m_d3d12NormalUploadBuffer.Get() != nullptr)
	{
		m_d3d12NormalUploadBuffer.Reset();
	}

	if (m_d3d12TangentUploadBuffer.Get() != nullptr)
	{
		m_d3d12TangentUploadBuffer.Reset();
	}

	if (m_d3d12BiTangentUploadBuffer.Get() != nullptr)
	{
		m_d3d12BiTangentUploadBuffer.Reset();
	}

	if (m_d3d12TexCoordUploadBuffer.Get() != nullptr)
	{
		m_d3d12TexCoordUploadBuffer.Reset();
	}

	if (!m_d3d12IndexUploadBuffers.empty())
	{
		for (auto& UploadBuffer : m_d3d12IndexUploadBuffers)
		{
			if (UploadBuffer.Get() != nullptr)
			{
				UploadBuffer.Reset();
			}
		}

		m_d3d12IndexUploadBuffers.shrink_to_fit();
	}
}

bool CMesh::CheckRayIntersection(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, const XMMATRIX& worldMatrix, float& hitDistance)
{
	bool intersected = false;

	// -1: BoundBox
	int subMeshCount = (int)m_indices.size() - 1;
	float nearestHitDistance = FLT_MAX;

	for (int i = 0; i < subMeshCount; ++i)
	{
		for (int j = 0; j < m_indices[i].size(); j += 3)
		{
			XMVECTOR vertex1 = XMVector3TransformCoord(XMLoadFloat3(&m_positions[m_indices[i][j]]), worldMatrix);
			XMVECTOR vertex2 = XMVector3TransformCoord(XMLoadFloat3(&m_positions[m_indices[i][j + 1]]), worldMatrix);
			XMVECTOR vertex3 = XMVector3TransformCoord(XMLoadFloat3(&m_positions[m_indices[i][j + 2]]), worldMatrix);

			// 메쉬의 모든 프리미티브(삼각형)들에 대하여 픽킹 광선과의 충돌을 검사한다.
			if (TriangleTests::Intersects(XMLoadFloat3(&rayOrigin), XMLoadFloat3(&rayDirection), vertex1, vertex2, vertex3, hitDistance))
			{
				if (hitDistance < nearestHitDistance)
				{
					nearestHitDistance = hitDistance;
					intersected = true;
				}
			}
		}
	}

	hitDistance = nearestHitDistance;

	return intersected;
}

void CMesh::Render(int subSetIndex)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12PositionBufferView, m_d3d12NormalBufferView, m_d3d12TangentBufferView, m_d3d12BiTangentBufferView, m_d3d12TexCoordBufferView };

	d3d12GraphicsCommandList->IASetVertexBuffers(0, _countof(vertexBufferViews), vertexBufferViews);
	d3d12GraphicsCommandList->IASetPrimitiveTopology(m_d3d12PrimitiveTopology);

	int bufferSize = static_cast<int>(m_d3d12IndexBuffers.size());

	if (bufferSize > 0 && subSetIndex < bufferSize)
	{
		d3d12GraphicsCommandList->IASetIndexBuffer(&m_d3d12IndexBufferViews[subSetIndex]);
		d3d12GraphicsCommandList->DrawIndexedInstanced(static_cast<UINT>(m_indices[subSetIndex].size()), 1, 0, 0, 0);
	}
	else
	{
		d3d12GraphicsCommandList->DrawInstanced(static_cast<UINT>(m_positions.size()), 1, 0, 0);
	}
}
