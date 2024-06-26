#include "pch.h"
#include "Terrain.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"
#include "Transform.h"

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		// 파일을 열지 못한 경우 예외 처리
		DWORD dwError = GetLastError();
		// 오류 처리 방법에 따라 적절한 조치를 취합니다.
		// 예를 들어, 오류 메시지 출력 또는 기타 처리를 수행할 수 있습니다.
		// 이 예제에서는 오류 코드를 출력하는 방식으로 처리합니다.
		std::cerr << "Failed to open file. Error code: " << dwError << std::endl;
		return; // 또는 적절한 처리 후 반환합니다.
	}
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

CTerrain::CTerrain(int nWidth, int nLength, int Type ) 
{
	SetActive(true);

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_type = Type;
	m_xmf3Scale = XMFLOAT3(1.f, 1.f, 1.f);

	// String to LPCTSTR
	string strPath;
	switch (m_type)
	{
	case TERRAIN_GROUND :
		strPath = CAssetManager::GetInstance()->GetAssetPath() + "Terrain\\VoteFightHeightMap.raw";
		break;
	case TERRAIN_SEA:
		strPath = CAssetManager::GetInstance()->GetAssetPath() + "Terrain\\SeaHeightMap.raw";
		
		break;
	}
	const char* cPath = strPath.c_str();
	wchar_t* wmsg = new wchar_t[strlen(cPath) + 1]; //memory allocation
	size_t nConverted = 0;
	mbstowcs_s(&nConverted, wmsg, strlen(cPath) + 1, cPath, _TRUNCATE);
	
	m_pHeightMapImage = new CHeightMapImage(wmsg, TERRAIN_WIDTH, TERRAIN_HEIGHT, m_xmf3Scale);
	delete[]wmsg;

	// m_pHeightMapImage = new CHeightMapImage(_T("C:\\directX_work\\VoteFight_new\\Release\\Asset\\Terrain\\FightVote_terrain.raw"), nWidth, nLength, m_xmf3Scale);
	MakeHeightMapGridMesh(0, 0, TERRAIN_WIDTH, TERRAIN_HEIGHT, m_xmf3Scale, m_pHeightMapImage);

	string strTerrain = "Terrain1";
	CMaterial* material = nullptr;
	CTexture* texture = nullptr;
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	switch (m_type)
	{
	case TERRAIN_GROUND:
		strTerrain = "Terrain1";
		material = CAssetManager::GetInstance()->CreateMaterial(strTerrain);
		texture = CAssetManager::GetInstance()->CreateTexture(strTerrain, "VoteFightTexture", TEXTURE_TYPE::ALBEDO_MAP);
		break;
	case TERRAIN_SEA:
		strTerrain = "Terrain2";
		material = CAssetManager::GetInstance()->CreateMaterial(strTerrain);
		texture = CAssetManager::GetInstance()->CreateTexture(strTerrain, "SeaTexture", TEXTURE_TYPE::ALBEDO_MAP);
		transform->SetPosition(XMFLOAT3(-1500, -100, -1500));
		transform->SetScale(XMFLOAT3(5, 1, 5));
		break;
	}
	CShader* shader = CAssetManager::GetInstance()->GetShader(strTerrain);

	material->SetTexture(texture);
	material->AddShader(shader);
	shader = CAssetManager::GetInstance()->GetShader("DepthWrite");
	material->AddShader(shader);

	material->SetStateNum(0);
	AddMaterial(material);

	
}

CTerrain::~CTerrain()
{
}

void CTerrain::MakeHeightMapGridMesh(int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, void* pContext)
{
	m_vertexCount = nWidth * nLength;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	XMFLOAT3* pxmf3Positions = new XMFLOAT3[m_vertexCount];
	XMFLOAT2* pxmf2TextureCoords = new XMFLOAT2[m_vertexCount];

	int cxHeightMap = m_nWidth;
	int czHeightMap = m_nLength;

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z);
			pxmf3Positions[i] = XMFLOAT3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			pxmf2TextureCoords[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	m_d3d12VertexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, pxmf3Positions, sizeof(XMFLOAT3) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12VertexUploadBuffer.GetAddressOf());
	m_d3d12VertexBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12VertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3d12VertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_vertexCount;

	m_d3d12TextureCoordBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, pxmf2TextureCoords, sizeof(XMFLOAT2) * m_vertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_d3d12TextureCoordUploadBuffer);
	m_d3d12TextureCoordBufferView.BufferLocation = m_d3d12TextureCoordBuffer->GetGPUVirtualAddress();
	m_d3d12TextureCoordBufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3d12TextureCoordBufferView.SizeInBytes = sizeof(XMFLOAT2) * m_vertexCount;

	m_indices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	UINT* pnSubSetIndices = new UINT[m_indices];

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) pnSubSetIndices[j++] = (UINT)(x + (z * nWidth));
				pnSubSetIndices[j++] = (UINT)(x + (z * nWidth));
				pnSubSetIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) pnSubSetIndices[j++] = (UINT)(x + (z * nWidth));
				pnSubSetIndices[j++] = (UINT)(x + (z * nWidth));
				pnSubSetIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_d3d12IndexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, pnSubSetIndices, sizeof(UINT) * m_indices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_d3d12IndexUploadBuffer.GetAddressOf());
	m_d3d12IndexBufferView.BufferLocation = m_d3d12IndexBuffer->GetGPUVirtualAddress();
	m_d3d12IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3d12IndexBufferView.SizeInBytes = sizeof(UINT) * m_indices;

	vector<XMFLOAT3> norVec3s;
	CreateNormalDate(pnSubSetIndices, pxmf3Positions, norVec3s);
	m_d3d12NormalBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, norVec3s.data(), sizeof(XMFLOAT3) * norVec3s.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12NormalUploadBuffer.GetAddressOf());
	m_d3d12NormalBufferView.BufferLocation = m_d3d12NormalBuffer->GetGPUVirtualAddress();
	m_d3d12NormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3d12NormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * norVec3s.size();
	cout << "사이즈" << norVec3s.size() << '\n';

}

float CTerrain::OnGetHeight(int x, int z)
{
	BYTE* pHeightMapPixels = m_pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = m_pHeightMapImage->GetScale();
	int nWidth = m_pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}

void CTerrain::CreateNormalDate(const UINT* pnSubSetIndices, const XMFLOAT3* vertices,  vector<XMFLOAT3>& new_NorVecs)
{
	
	for (int j = 0, z = 0; z < m_nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < m_nWidth; x++)
			{
				new_NorVecs.push_back(m_pHeightMapImage->GetHeightMapNormal(x, z));
			}
		}
		else
		{
			for (int x = m_nWidth - 1; x >= 0; x--)
			{
				new_NorVecs.push_back(m_pHeightMapImage->GetHeightMapNormal(x, z));
			}
		}
	}

	/*
	for (size_t i = 0; i < m_indices/3; i++)
	{
		UINT index0 = pnSubSetIndices[i * 3 + 0];
		UINT index1 = pnSubSetIndices[i * 3 + 1];
		UINT index2 = pnSubSetIndices[i * 3 + 2];

		XMFLOAT3 v0 = vertices[index0];
		XMFLOAT3 v1 = vertices[index1];
		XMFLOAT3 v2 = vertices[index2];

		XMFLOAT3 a = XMFLOAT3( v0.x - v2.x , v0.y - v2.y, v0.z - v2.z);
		XMFLOAT3 b = XMFLOAT3( v0.x - v1.x , v0.y - v1.y, v0.z - v1.z);


		XMFLOAT3 normal;
		normal = Vector3::CrossProduct(a, b);
		new_NorVecs.push_back(normal);
	}
	*/
}

void CTerrain::PreRender(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	
	XMFLOAT4X4 xmf4x4world = Matrix4x4::Identity();
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &xmf4x4world, 0);

	CMaterial* material = GetMaterials()[0];
	material->SetPipelineState(RENDER_TYPE::DEPTH_WRITE);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView, m_d3d12TextureCoordBufferView, m_d3d12NormalBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 3, vertexBufferViews);

	d3d12GraphicsCommandList->IASetIndexBuffer(&m_d3d12IndexBufferView);
	d3d12GraphicsCommandList->DrawIndexedInstanced(m_indices, 1, 0, 0, 0);
}

void CTerrain::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView, m_d3d12TextureCoordBufferView, m_d3d12NormalBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 3, vertexBufferViews);

	CMaterial* material = GetMaterials()[0];

	material->SetPipelineState(RENDER_TYPE::STANDARD);
	XMFLOAT4X4 xmf4x4world = Matrix4x4::Identity();
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &xmf4x4world, 0);
	material->UpdateShaderVariables();

	d3d12GraphicsCommandList->IASetIndexBuffer(&m_d3d12IndexBufferView);
	d3d12GraphicsCommandList->DrawIndexedInstanced(m_indices, 1, 0, 0, 0);
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