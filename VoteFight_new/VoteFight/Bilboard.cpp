#include "pch.h"
#include "Bilboard.h"

#include "GameFramework.h"

#include "AssetManager.h"

#include "Texture.h"
#include "Material.h"

CBilboard::CBilboard() :
	m_maxVertexCount(),
	m_vertexCount(),
	m_d3d12VertexBuffer(),
	m_d3d12VertexUploadBuffer(),
	m_d3d12VertexBufferView(),
	m_mappedQuadInfo()
{
}

CBilboard::~CBilboard()
{
}

void CBilboard::ReleaseUploadBuffers()
{
	if (m_d3d12VertexUploadBuffer.Get() != nullptr)
	{
		m_d3d12VertexUploadBuffer.Reset();
	}

	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		child->ReleaseUploadBuffers();
	}
}

void CBilboard::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 1, vertexBufferViews);

	const vector<CMaterial*>& materials = GetMaterials();

	for (int i = 0; i < materials.size(); ++i)
	{
		materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
		materials[i]->UpdateShaderVariables();

		d3d12GraphicsCommandList->DrawInstanced(m_vertexCount, 1, 0, 0);
	}

	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}

//=========================================================================================================================

CSkyBox::CSkyBox()
{
	SetActive(true);

	m_maxVertexCount = m_vertexCount = 6;

	vector<QuadInfo> vertices(m_maxVertexCount);

	// Left
	vertices[0].m_position = XMFLOAT3(-10.0f, 0.0f, 0.0f);
	vertices[0].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[0].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Right
	vertices[1].m_position = XMFLOAT3(+10.0f, 0.0f, 0.0f);
	vertices[1].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[1].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Bottom
	vertices[2].m_position = XMFLOAT3(0.0f, -10.0f, 0.0f);
	vertices[2].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[2].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Top
	vertices[3].m_position = XMFLOAT3(0.0f, +10.0f, 0.0f);
	vertices[3].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[3].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Back
	vertices[4].m_position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	vertices[4].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[4].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Front
	vertices[5].m_position = XMFLOAT3(0.0f, 0.0f, +10.0f);
	vertices[5].m_size = XMFLOAT2(20.0f, 20.0f);
	vertices[5].m_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	m_d3d12VertexBuffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, vertices.data(), sizeof(QuadInfo) * m_maxVertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_d3d12VertexUploadBuffer.GetAddressOf());
	m_d3d12VertexBufferView.BufferLocation = m_d3d12VertexBuffer->GetGPUVirtualAddress();
	m_d3d12VertexBufferView.StrideInBytes = sizeof(QuadInfo);
	m_d3d12VertexBufferView.SizeInBytes = sizeof(QuadInfo) * m_maxVertexCount;

	// Left
	CMaterial* material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Left");
	CTexture* texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Left");
	CShader* shader = CAssetManager::GetInstance()->GetShader("Bilboard");

	texture->Load("SkyBox_Left", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);

	// Right
	material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Right");
	texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Right");
	texture->Load("SkyBox_Right", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);

	// Bottom
	material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Bottom");
	texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Bottom");
	texture->Load("SkyBox_Bottom", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);

	// Top
	material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Top");
	texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Top");
	texture->Load("SkyBox_Top", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);

	// Back
	material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Back");
	texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Back");
	texture->Load("SkyBox_Back", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);

	// Front
	material = CAssetManager::GetInstance()->CreateMaterial("SkyBox_Front");
	texture = CAssetManager::GetInstance()->CreateTexture("SkyBox_Front");
	texture->Load("SkyBox_Front", TEXTURE_TYPE::ALBEDO_MAP);
	material->SetTexture(texture);
	material->AddShader(shader);
	material->SetStateNum(1);
	AddMaterial(material);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(CCamera* camera)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = { m_d3d12VertexBufferView };

	d3d12GraphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	d3d12GraphicsCommandList->IASetVertexBuffers(0, 1, vertexBufferViews);

	const vector<CMaterial*>& materials = GetMaterials();

	for (int i = 0; i < materials.size(); ++i)
	{
		materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
		materials[i]->UpdateShaderVariables();

		d3d12GraphicsCommandList->DrawInstanced(1, 1, i, 0);
	}

	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}