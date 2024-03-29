#include "pch.h"
#include "Terrain.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}
	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2);

	return(xmf3Normal);
}

CTerrain::CTerrain(LPCTSTR pFileName, int nWidth = 100.f, int nLength = 100.f, XMFLOAT3 xmf3Scale = XMFLOAT3(5.f, 5.f, 5.f))
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
	MakeHeightMapGridMesh(0, 0, nWidth, nLength,  xmf3Scale, m_pHeightMapImage);
}

CTerrain::~CTerrain()
{
}

void CTerrain::MakeHeightMapGridMesh(int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, void* pContext)
{
	m_maxVertexCount = m_vertexCount = nWidth * nLength;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	XMFLOAT3* m_pxmf3Positions = new XMFLOAT3[m_maxVertexCount];
	XMFLOAT2* m_pxmf2TextureCoords0 = new XMFLOAT2[m_maxVertexCount];

	int cxHeightMap = m_nWidth;
	int czHeightMap = m_nLength;

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);
			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	m_d3d12VertexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12VertexUploadBuffer.GetAddressOf());
	m_d3d12VertexBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12VertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3d12VertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_maxVertexCount;

	m_d3d12TextureCoord0Buffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_d3d12TextureCoord0UploadBuffer);
	m_d3d12TextureCoord0BufferView.BufferLocation = m_d3d12TextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3d12TextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3d12TextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_maxVertexCount;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

	m_pnSubSetIndices[0] = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_ppnSubSetIndices[0] = new UINT[m_pnSubSetIndices[0]];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)(x + (z * nWidth));
				m_ppnSubSetIndices[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_d3d12IndexBuffers[0] = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_d3d12IndexUploadBuffers[0].GetAddressOf());
	m_d3d12IndexBufferViews[0].BufferLocation = m_d3d12IndexBuffers[0]->GetGPUVirtualAddress();
	m_d3d12IndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3d12IndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];



	string strTerrain = "Terrain";
	CMaterial* material = CAssetManager::GetInstance()->CreateMaterial(strTerrain);
	CTexture* sub_texture = CAssetManager::GetInstance()->CreateTexture(strTerrain);
	CTexture* texture = CAssetManager::GetInstance()->CreateTexture(strTerrain);
	CShader* shader = CAssetManager::GetInstance()->GetShader(strTerrain);
}

float CTerrain::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}


void CTerrain::Render(CCamera* camera, int subSetIndex)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertex_and_Texture_BufferViews[] = { m_d3d12VertexBufferView,m_d3d12TextureCoord0BufferView };

	vector<D3D12_INDEX_BUFFER_VIEW> IndexBufferViews[] = { m_d3d12IndexBufferViews };


	// =================================================================================

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3d12GraphicsCommandList->IASetVertexBuffers(0, 2, vertex_and_Texture_BufferViews);

	d3d12GraphicsCommandList->IASetIndexBuffer(IndexBufferViews[0].data());

	d3d12GraphicsCommandList->DrawIndexedInstanced(static_cast<UINT>(IndexBufferViews[subSetIndex].size()), 1, 0, 0, 0);


	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}

XMFLOAT4 CTerrain::Add(const XMFLOAT4& vec0, const XMFLOAT4& vec1)
{
	XMFLOAT4 addVector;
	addVector.x = vec0.x + vec1.x;
	addVector.y = vec0.y + vec1.y;
	addVector.z = vec0.z + vec1.z;
	addVector.w = vec0.w + vec1.w;
	return addVector;
}
