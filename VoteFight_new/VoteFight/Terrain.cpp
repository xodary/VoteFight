#include "pch.h"
#include "Terrain.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"

CTerrain::CTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, 
	int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	m_xmf3Scale = xmf3Scale;

	LoadHeightMap(pFileName, nWidth, nLength, xmf3Scale);
	MakeHeightMapGridMesh(pd3dDevice, pd3dCommandList, 0, 0, nWidth, nLength, xmf3Scale, xmf4Color, m_pHeightMapPixels);


}

CTerrain::~CTerrain()
{
}

void CTerrain::LoadHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
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

void CTerrain::MakeHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext)
{
	m_maxVertexCount = m_vertexCount = nWidth * nLength;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	XMFLOAT3* m_pxmf3Positions = new XMFLOAT3[m_maxVertexCount];
	XMFLOAT4* m_pxmf4Colors = new XMFLOAT4[m_maxVertexCount];
	XMFLOAT2* m_pxmf2TextureCoords0 = new XMFLOAT2[m_maxVertexCount];
	XMFLOAT2* m_pxmf2TextureCoords1 = new XMFLOAT2[m_maxVertexCount];

	int cxHeightMap = m_nWidth;
	int czHeightMap = m_nLength;

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			//fHeight = OnGetHeight(x, z, pContext);
			m_pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			//m_pxmf4Colors[i] = XMFLOAT4(OnGetColor(x, z, pContext), xmf4Color);
			m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x*0.5f), float(z) / float(m_xmf3Scale.z*0.5f));
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

	m_d3d12ColorBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12ColorUploadBuffer.GetAddressOf());
	m_d3d12ColorBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12ColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3d12ColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_maxVertexCount;

	m_d3d12TextureCoord0Buffer = DX::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_d3d12TextureCoord0UploadBuffer);
	m_d3d12TextureCoord0BufferView.BufferLocation = m_d3d12TextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3d12TextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3d12TextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_vertexCount;

	m_d3d12TextureCoord1Buffer = DX::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_d3d12TextureCoord1UploadBuffer);
	m_d3d12TextureCoord1BufferView.BufferLocation = m_d3d12TextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3d12TextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3d12TextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_vertexCount;

	m_nSubMeshes = 1;
	m_pnSubSetIndices = new int[m_nSubMeshes];
	m_ppnSubSetIndices = new UINT*[m_nSubMeshes];

	m_d3d12SubSetIndexBuffers.emplace_back(m_nSubMeshes);
	m_d3d12SetIndexUploadBuffers.emplace_back(m_nSubMeshes);
	m_d3d12SubSetIndexBufferViews.emplace_back(m_nSubMeshes);

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

	m_d3d12SubSetIndexBuffers[0] = DX::CreateBufferResource(pd3dDevice, pd3dCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_d3d12SetIndexUploadBuffers[0]);

	m_d3d12SubSetIndexBufferViews[0].BufferLocation = m_d3d12SubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_d3d12SubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3d12SubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];
	
	m_d3d12IndexBuffers[0] = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_ppnSubSetIndices[0], sizeof(UINT) * m_pnSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_d3d12IndexUploadBuffers[0].GetAddressOf());
	m_d3d12IndexBufferViews[0].BufferLocation = m_d3d12IndexBuffers[0]->GetGPUVirtualAddress();
	m_d3d12IndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3d12IndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_pnSubSetIndices[0];

	string strTerrain = "Terrain";
	CMaterial* material = CAssetManager::GetInstance()->CreateMaterial(strTerrain);
	CTexture* texture = CAssetManager::GetInstance()->CreateTexture(strTerrain);
	CShader* shader = CAssetManager::GetInstance()->GetShader(strTerrain);
}

float CTerrain::GetHeight(float fx, float fz, bool bReverseQuad)
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


void CTerrain::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 1, vertexBufferViews);

	CMaterial* material = GetMaterials()[0];

	material->SetPipelineState(RENDER_TYPE::STANDARD);
	XMFLOAT4X4 xmf4x4world = Matrix4x4::Identity();
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &xmf4x4world, 0);
	material->UpdateShaderVariables();

	d3d12GraphicsCommandList->DrawInstanced(m_maxVertexCount, 1, 0, 0);

	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}
