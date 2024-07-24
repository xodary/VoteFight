#include "pch.h"
#include "AssetManager.h"
#include "DescriptorHeap.h"
#include "GameFramework.h"
#include "SkinnedMesh.h"
#include "Texture.h"
#include "ObjectShader.h"
#include "WireFrameShader.h"
#include "SkyboxShader.h"
#include "UIShader.h"
#include "BilboardShader.h"
#include "Material.h"
#include "Animation.h"
#include "DepthWriteShader.h"
#include "GameFramework.h"
#include "VoxelizationShader.h"
#include "AnisoMipmap.h"
#include "GBufferShader.h"
#include "UpsampleBlur.h"
#include "VCTMainShader.h"
#include "MainShader.h"

CAssetManager::CAssetManager() :
	m_assetPath(),
	m_meshes(),
	m_textures(),
	m_shaders(),
	m_materials(),
	m_animations()
{
}

CAssetManager::~CAssetManager()
{
	Utility::SafeDelete(m_meshes);
	Utility::SafeDelete(m_textures);
	Utility::SafeDelete(m_shaders);
	Utility::SafeDelete(m_materials);

	for (auto& p : m_animations)
	{
		Utility::SafeDelete(p.second);
	}
}

void CAssetManager::LoadMeshes(const string& fileName)
{
	string filePath = m_assetPath + "Mesh\\" + fileName;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << filePath << " : ������ �� �� �����ϴ�." << std::endl;
		return;
	}

	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Meshes>")
		{
			int meshCount = 0;

			in.read(reinterpret_cast<char*>(&meshCount), sizeof(int));

			if (meshCount > 0)
			{
				m_meshes.reserve(meshCount);
				cout << fileName << " �ε� ����...\n";
			}
		}
		else if (str == "<Mesh>")
		{
			CMesh* mesh = new CMesh();

			mesh->Load(in);
			m_meshes.emplace(mesh->GetName(), mesh);
		}
		else if (str == "<SkinnedMesh>")
		{
			// <RefName>
			File::ReadStringFromFile(in, str);
			File::ReadStringFromFile(in, str);

			CSkinnedMesh* skinnedMesh = new CSkinnedMesh(*GetMesh(str));

			skinnedMesh->LoadSkinInfo(in);
			m_meshes[skinnedMesh->GetName()] = skinnedMesh;
		}
		else if (str == "</Meshes>")
		{
			cout << fileName << " �ε� �Ϸ�...(�޽� ����: " << m_meshes.size() << ")\n\n";
			break;
		}
	}

	m_meshes["HealthBar"] = new CRectMesh(1.5, 0.1);
	m_meshes["SpeechBubble"] = new CRectMesh(2.5, BUBBLE_HEIGHT);
}

void CAssetManager::LoadTextures(const string& fileName)
{
	string filePath = m_assetPath + "Texture\\" + fileName;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << filePath << " : ������ �� �� �����ϴ�." << std::endl;
		return;
	}

	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Textures>")
		{
			int textureCount = 0;

			in.read(reinterpret_cast<char*>(&textureCount), sizeof(int));

			if (textureCount > 0)
			{
				m_textures.reserve(textureCount);
				cout << fileName << " �ε� ����...\n";
			}
		}
		else if (str == "<Texture>")
		{
			CTexture* texture = new CTexture();

			texture->Load(in);
			m_textures.emplace(texture->GetName(), texture);
		}
		else if (str == "</Textures>")
		{
			cout << fileName << " �ε� �Ϸ�...(�ؽ�ó ����: " << m_textures.size() << ")\n\n";
			break;
		}
	}

	CGameFramework* framework = CGameFramework::GetInstance();
	ID3D12Device* device = framework->GetDevice();
	ID3D12GraphicsCommandList* commandList = framework->GetGraphicsCommandList();
	DescriptorHeapManager* descriptorHeapManager = framework->GetDescriptorHeapManager();

	// DepthWrite Texture
	const XMFLOAT2& resolution = framework->GetResolution();
	CTexture* texture = new CTexture();

	texture->SetName("DepthWrite");
	texture->Create(static_cast<UINT64>(DEPTH_BUFFER_WIDTH), static_cast<UINT>(DEPTH_BUFFER_HEIGHT), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, DXGI_FORMAT_R32_FLOAT, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R32_FLOAT, { 1.0f, 1.0f, 1.0f, 1.0f } }, TEXTURE_TYPE::SHADOW_MAP);
	m_textures.emplace(texture->GetName(), texture);

	// GBuffer Texture
	CTexture* GBufferColor = new CTexture();
	CTexture* GBufferNormal = new CTexture();
	CTexture* GBufferWorldPos = new CTexture();

	GBufferColor->SetName("GBufferColor");
	GBufferNormal->SetName("GBufferNormal");
	GBufferWorldPos->SetName("GBufferWorldPos");

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.Format = format;
	GBufferColor->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::G_COLOR);
	GBufferColor->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GBufferColor->GetTexture(), &rtvDesc, GBufferColor->m_RTVHandle.GetCPUHandle());

	format = DXGI_FORMAT_R16G16B16A16_SNORM;
	rtvDesc.Format = format;
	GBufferNormal->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::G_NORMAL);
	GBufferNormal->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GBufferNormal->GetTexture(), &rtvDesc, GBufferNormal->m_RTVHandle.GetCPUHandle());

	format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.Format = format;
	GBufferWorldPos->Create(static_cast<UINT64>(framework->GetResolution().x), static_cast<UINT>(framework->GetResolution().y), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, format, D3D12_CLEAR_VALUE{ format, { 0.0f, 0.0f, 0.0f, 0.0f } }, TEXTURE_TYPE::G_WORLDPOS);
	GBufferWorldPos->m_RTVHandle = descriptorHeapManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GBufferWorldPos->GetTexture(), &rtvDesc, GBufferWorldPos->m_RTVHandle.GetCPUHandle());

	m_textures.emplace(GBufferColor->GetName(), GBufferColor);
	m_textures.emplace(GBufferNormal->GetName(), GBufferNormal);
	m_textures.emplace(GBufferWorldPos->GetName(), GBufferWorldPos);

	CTexture* textTexture = new CTexture();
	textTexture->SetName("text");
	textTexture->Load("text", TEXTURE_TYPE::ALBEDO_MAP);
	m_textures.emplace(textTexture->GetName(), textTexture);

	CTexture* bubbleTexture = new CTexture();
	bubbleTexture->SetName("speech_bubble");
	bubbleTexture->Load("speech_bubble", TEXTURE_TYPE::ALBEDO_MAP);
	m_textures.emplace(bubbleTexture->GetName(), bubbleTexture);

	CTexture* IconsTexture = new CTexture();
	IconsTexture->SetName("Icons");
	IconsTexture->Load("Icons", TEXTURE_TYPE::ALBEDO_MAP);
	m_textures.emplace(IconsTexture->GetName(), IconsTexture);

	// Icons
	string itemName[] = { "axe", "wood", "punch", "money",
		"election_ticket", "fish_meet", "potato", "trash",
		"icecream", "carrot", "drug", "gun", "flower", 
		"arrow", "space", "letter_f", "letter_e", "letter_q" };

	for (auto str : itemName)
	{
		CTexture* ex = new CTexture();
		ex->SetName(str);
		ex->Load(str, TEXTURE_TYPE::ALBEDO_MAP);
		m_textures.emplace(ex->GetName(), ex);
		m_IconTextures[str] = ex;
	}

	string MiniMapIconNames[] = { "MinimapIcons/GrayNPC",  "MinimapIcons/RadPlayer",
		"MinimapIcons/WeaponIcon" , "MinimapIcons/ItemIcon" , "MinimapIcons/ItemIcon" ,
	"MinimapIcons/MonsterIcon" , "MinimapIcons/MiniMap" };

	for (auto str : MiniMapIconNames)
	{
		CTexture* ex = new CTexture();
		ex->Load(str, TEXTURE_TYPE::ALBEDO_MAP);
		m_IconTextures[str] = ex;
	}

}

void CAssetManager::LoadShaders()
{
	CShader* shader = new CObjectShader();
	shader->SetName("Object");
	shader->CreatePipelineStates(3);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CSkyboxShader();
	shader->SetName("Skybox");
	shader->CreatePipelineStates(1);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CUIShader();
	shader->SetName("UI");
	shader->CreatePipelineStates(2);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CWireFrameShader();
	shader->SetName("WireFrame");
	shader->CreatePipelineStates(1);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CBilboardShader();
	shader->SetName("Bilboard");
	shader->CreatePipelineStates(2);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CMainShader();
	shader->SetName("MainShader");
	shader->CreatePipelineStates(1);
	m_shaders.emplace(shader->GetName(), shader);

	shader = new CDepthWriteShader();
	shader->SetName("DepthWrite");
	shader->CreatePipelineStates(2);
	m_shaders.emplace(shader->GetName(), shader);

}

void CAssetManager::LoadMaterials(const string& fileName)
{
	string filePath = m_assetPath + "Material\\" + fileName;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << "������ �� �� �����ϴ�." << std::endl;
		return;
	}
	cout << fileName << "  �ε�..." << endl;
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Materials>")
		{
			int materialCount = 0;

			in.read(reinterpret_cast<char*>(&materialCount), sizeof(int));

			if (materialCount > 0)
			{
				m_materials.reserve(materialCount);
				cout << fileName << " �ε� ����...\n";
			}
		}
		else if (str == "<Material>")
		{
			CMaterial* material = new CMaterial();

			material->Load(in);
			m_materials.emplace(material->GetName(), material);
		}
		else if (str == "</Materials>")
		{
			cout << fileName << " �ε� �Ϸ�...(���͸��� ����: " << m_materials.size() << ")\n\n";
			break;
		}
	}
}

void CAssetManager::LoadSkinningAnimations(const string& fileName)
{
	string modelName = fileName;

	if (m_animations.find(modelName) == m_animations.end())
	{
		string filePath = m_assetPath + "Animation\\" + fileName + "_Animation.bin";
		ifstream in(filePath, ios::binary);
		string str;

		while (true)
		{
			File::ReadStringFromFile(in, str);

			if (str == "<Animations>")
			{
				int animationCount = 0;

				in.read(reinterpret_cast<char*>(&animationCount), sizeof(int));
				m_animations[modelName].reserve(animationCount);
				cout << fileName << " �ִϸ��̼� �ε� ����...\n";
			}
			else if (str == "<Animation>")
			{
				CAnimation* skinningAnimation = new CSkinningAnimation();

				skinningAnimation->Load(in);
				m_animations[modelName].push_back(skinningAnimation);
			}
			else if (str == "</Animations>")
			{
				cout << fileName << " �ִϸ��̼� �ε� �Ϸ�...\n";
				break;
			}
		}
	}
}

void CAssetManager::LoadUIAnimations(ifstream& in, const string& key)
{
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Animations>")
		{
			int animationCount = 0;

			in.read(reinterpret_cast<char*>(&animationCount), sizeof(int));
			m_animations[key].reserve(animationCount);
			cout << key << " �ִϸ��̼� �ε� ����...\n";
		}
		else if (str == "<Animation>")
		{
			CAnimation* uiAnimation = new CUIAnimation();

			uiAnimation->Load(in);
			m_animations[key].push_back(uiAnimation);
		}
		else if (str == "</Animations>")
		{
			cout << key << " �ִϸ��̼� �ε� �Ϸ�...\n";
			break;
		}
	}
}

const string& CAssetManager::GetAssetPath()
{
	return m_assetPath;
}

CMesh* CAssetManager::GetMesh(string& key)
{
	CMesh* mesh = nullptr;

	int num = key.find("_Instance");
		
	if (num != string::npos) {
		key = key.substr(0, num);
	}

	if (m_meshes.find(key) != m_meshes.end()) {
		mesh = m_meshes[key];
	}

	return mesh;
}

int CAssetManager::GetMeshCount()
{
	return static_cast<int>(m_meshes.size());
}

CTexture* CAssetManager::CreateTexture(const string& key, const string& textureName, const TEXTURE_TYPE& type)
{
	CTexture* texture = GetTexture(key);

	if (texture == nullptr)
	{
		texture = new CTexture();
		texture->SetName(key);
		texture->Load(textureName, type);
		m_textures.emplace(key, texture);
	}

	return texture;
}

CTexture* CAssetManager::GetTexture(const string& key)
{
	CTexture* texture = nullptr;

	if (m_textures.find(key) != m_textures.end())
	{
		texture = m_textures[key];
	}

	return texture;
}

int CAssetManager::GetTextureCount()
{
	return static_cast<int>(m_textures.size());
}

CShader* CAssetManager::GetShader(const string& key)
{
	CShader* shader = nullptr;

	if (m_shaders.find(key) != m_shaders.end())
	{
		shader = m_shaders[key];
	}

	return shader;
}

int CAssetManager::GetShaderCount()
{
	return static_cast<int>(m_shaders.size());
}

CMaterial* CAssetManager::CreateMaterial(string& key)
{
	CMaterial* material = GetMaterial(key);
	
	if (material == nullptr)
	{
		material = new CMaterial();
		material->SetName(key);
		m_materials.emplace(key, material);
	}

	material = CreateMaterialInstance(key);

	return material;
}

CMaterial* CAssetManager::CreateMaterialInstance(string& key)
{
	CMaterial* material = GetMaterial(key);

	// �ش� key�� ���� ���͸����� �����ϸ�, ���� �����ڷ� �ν��Ͻ��� ������ �Ŀ� ��ȯ�Ѵ�.
	if (material != nullptr)
	{
		material = new CMaterial(*material);
		material->SetName(material->GetName() + "_Instance");
	}

	return material;
}

CMaterial* CAssetManager::GetMaterial(string& key)
{
	CMaterial* material = nullptr;

	int num = key.find("_(Instance)");

	if (num != string::npos) {
		key = key.substr(0, num);
	}

	if (m_materials.find(key) != m_materials.end())
	{
		material = m_materials[key];
	}

	return material;
}

int CAssetManager::GetMaterialCount()
{
	return static_cast<int>(m_materials.size());
}

const vector<CAnimation*>& CAssetManager::GetAnimations(const string& key)
{
	return m_animations[key];
}

int CAssetManager::GetAnimationCount(const string& key)
{
	return static_cast<int>(m_animations[key].size());
}

void CAssetManager::Init()
{
	char assetPath[255] = {};

	GetCurrentDirectoryA(255, assetPath);

	for (int i = static_cast<int>(strlen(assetPath) - 1); i >= 0; --i)
	{
		if (assetPath[i] == '\\')
		{
			assetPath[i] = '\0';

			break;
		}
	}

	strcat_s(assetPath, 255, "\\Release\\Asset\\");
	m_assetPath = assetPath;

	SceneLoadTextures();
	SceneLoadMeshes();
	LoadShaders();
	SceneLoadMaterials();

	// DepthWrite
	CTexture* texture = CAssetManager::GetInstance()->GetTexture("DepthWrite");
	D3D12_RENDER_TARGET_VIEW_DESC d3d12RenderTargetViewDesc = {};

	d3d12RenderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	d3d12RenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3d12RenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3d12RenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	CGameFramework* framework = CGameFramework::GetInstance();
	texture->m_RTVHandle = framework->GetDescriptorHeapManager()->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	framework->GetDevice()->CreateRenderTargetView(texture->GetTexture(), &d3d12RenderTargetViewDesc, texture->m_RTVHandle.GetCPUHandle());

	D3D12_DEPTH_STENCIL_VIEW_DESC d3d12DepthStencilViewDesc = {};

	d3d12DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	d3d12DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3d12DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	texture->m_DSVHandle = framework->GetDescriptorHeapManager()->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	texture->m_DepthStencilResource = DX::CreateTextureResource(framework->GetDevice(), DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT, 1, 1, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, DXGI_FORMAT_D32_FLOAT, D3D12_CLEAR_VALUE{ DXGI_FORMAT_D32_FLOAT, {1.0f, 0.0f} });
	framework->GetDevice()->CreateDepthStencilView(texture->m_DepthStencilResource.Get(), &d3d12DepthStencilViewDesc, texture->m_DSVHandle.GetCPUHandle());
}

void CAssetManager::CreateShaderResourceViews()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	DescriptorHeapManager* descriptorManager = CGameFramework::GetInstance()->GetDescriptorHeapManager();

	for (const auto& texture : m_textures)
	{
		texture.second->m_SRVHandle = descriptorManager->CreateCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		ID3D12Resource* pShaderResource = texture.second->GetTexture();
		if (((texture.second)->GetType() == TEXTURE_TYPE::CUBE_MAP) ||	// Cubemap (Skybox) 
			((texture.second)->GetType() == TEXTURE_TYPE::CUBE_MAP2)) {	// Cubemap (Skybox) 
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
			d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			d3dShaderResourceViewDesc.Format = pShaderResource->GetDesc().Format;
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
			d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;

			d3d12Device->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, texture.second->m_SRVHandle.GetCPUHandle());
		}
		else
			d3d12Device->CreateShaderResourceView(pShaderResource, nullptr, texture.second->m_SRVHandle.GetCPUHandle());
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = CGameFramework::GetInstance()->m_GUISrvDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = CGameFramework::GetInstance()->m_GUISrvDescHeap->GetGPUDescriptorHandleForHeapStart();

	for (auto& texture : m_IconTextures)
	{
		cpuHandle.ptr += CGameFramework::GetInstance()->m_srvDescriptorIncrementSize;
		gpuHandle.ptr += CGameFramework::GetInstance()->m_srvDescriptorIncrementSize;

		texture.second->m_IconGPUHandle = gpuHandle;
		d3d12Device->CreateShaderResourceView(texture.second->GetTexture(), nullptr, cpuHandle);
	}
}

void CAssetManager::ReleaseUploadBuffers()
{
	for (const auto& p : m_meshes)
	{
		p.second->ReleaseUploadBuffers();
	}

	for (const auto& p : m_textures)
	{
		p.second->ReleaseUploadBuffers();
	}
}

void CAssetManager::SceneLoadMeshes()
{
	LoadMeshes("Meshes.bin");
	LoadMeshes("WeaponMeshes.bin");
	LoadMeshes("PlayerMeshes.bin");
	LoadMeshes("Meshes2.bin");
}

void CAssetManager::SceneLoadMaterials()
{
	LoadMaterials("Materials.bin");
	LoadMaterials("WeaponMaterials.bin");
	LoadMaterials("PlayerMaterials.bin");
	LoadMaterials("Materials2.bin");
}

void CAssetManager::SceneLoadTextures()
{
	LoadTextures("Textures.bin");
	LoadTextures("WeaponTextures.bin");
	LoadTextures("PlayerTextures.bin");
	LoadTextures("Textures2.bin");
}
