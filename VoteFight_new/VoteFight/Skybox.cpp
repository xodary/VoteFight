#include "pch.h"
#include "Skybox.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "Texture.h"
#include "Material.h"

CSkyBox::CSkyBox(float fWidth, float fHeight, float fDepth)
{
	SetActive(true);

	m_maxVertexCount = m_vertexCount = 36;

	XMFLOAT3* m_pxmf3Positions = new XMFLOAT3[m_maxVertexCount];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	m_d3d12VertexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, m_pxmf3Positions, sizeof(XMFLOAT3) * m_maxVertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12VertexUploadBuffer.GetAddressOf());
	m_d3d12VertexBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12VertexBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3d12VertexBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_maxVertexCount;

	string strSkybox = "Skybox";
	CMaterial* material = CAssetManager::GetInstance()->CreateMaterial(strSkybox);
	CTexture* texture = CAssetManager::GetInstance()->CreateTexture(strSkybox, "SkyBox_1", TEXTURE_TYPE::CUBE_MAP);
	material->SetTexture(texture); 
	CTexture* texture1 = CAssetManager::GetInstance()->CreateTexture(strSkybox, "SkyBox_2", TEXTURE_TYPE::CUBE_MAP2);
	material->SetTexture(texture1);
	CShader* shader = CAssetManager::GetInstance()->GetShader(strSkybox);
	
	material->AddShader(shader);
	material->SetStateNum(0);
	AddMaterial(material);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(CCamera* camera)
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
}