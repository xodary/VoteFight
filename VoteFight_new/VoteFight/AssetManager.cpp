#include "pch.h"
#include "AssetManager.h"

#include "GameFramework.h"

#include "SkinnedMesh.h"
#include "Texture.h"
#include "ObjectShader.h"
#include "DepthWriteShader.h"
#include "BilboardShader.h"
#include "UIShader.h"
#include "WireFrameShader.h"
#include "Material.h"
#include "Animation.h"

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
			m_meshes.emplace(skinnedMesh->GetName(), skinnedMesh);
		}
		else if (str == "</Meshes>")
		{
			cout << fileName << " �ε� �Ϸ�...(�޽� ����: " << m_meshes.size() << ")\n\n";
			break;
		}
	}
}

void CAssetManager::LoadTextures(const string& fileName)
{
	string filePath = m_assetPath + "Texture\\" + fileName;
	ifstream in(filePath, ios::binary);
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

	// DepthWrite Texture
	const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();
	CTexture* texture = new CTexture();

	texture->SetName("DepthWrite");
	texture->Create(static_cast<UINT64>(DEPTH_BUFFER_WIDTH), static_cast<UINT>(DEPTH_BUFFER_HEIGHT), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, DXGI_FORMAT_R32_FLOAT, D3D12_CLEAR_VALUE{ DXGI_FORMAT_R32_FLOAT, { 1.0f, 1.0f, 1.0f, 1.0f } }, TEXTURE_TYPE::SHADOW_MAP);
	m_textures.emplace(texture->GetName(), texture);

	// PostProcessing Texture
}

void CAssetManager::LoadShaders()
{
	//// �������� �ʿ��� ���̴� ��ü(PSO)�� �����Ѵ�.
	//CShader* shader = new CDepthWriteShader();

	//shader->SetName("DepthWrite");
	//shader->CreatePipelineStates(3);
	//m_shaders.emplace(shader->GetName(), shader);

	CShader* shader = new CObjectShader();
	shader->SetName("Object");
	shader->CreatePipelineStates(2);
	m_shaders.emplace(shader->GetName(), shader);

	//shader = new CBilboardShader();
	//shader->SetName("Bilboard");
	//shader->CreatePipelineStates(2);
	//m_shaders.emplace(shader->GetName(), shader);

	//shader = new CUIShader();
	//shader->SetName("UI");
	//shader->CreatePipelineStates(2);
	//m_shaders.emplace(shader->GetName(), shader);

	//shader = new CWireFrameShader();
	//shader->SetName("WireFrame");
	//shader->CreatePipelineStates(1);
	//m_shaders.emplace(shader->GetName(), shader);
}

void CAssetManager::LoadMaterials(const string& fileName)
{
	string filePath = m_assetPath + "Material\\" + fileName;
	ifstream in(filePath, ios::binary);
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
	// str.length() - 14 : _Animation.bin
	string modelName = fileName.substr(0, fileName.length() - 14);

	if (m_animations.find(modelName) == m_animations.end())
	{
		string filePath = m_assetPath + "Animation\\" + fileName;
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

CMesh* CAssetManager::GetMesh(const string& key)
{
	CMesh* mesh = nullptr;

	if (m_meshes.find(key) != m_meshes.end())
	{
		mesh = m_meshes[key];
	}

	return mesh;
}

int CAssetManager::GetMeshCount()
{
	return static_cast<int>(m_meshes.size());
}

CTexture* CAssetManager::CreateTexture(const string& key)
{
	CTexture* texture = GetTexture(key);

	if (texture == nullptr)
	{
		texture = new CTexture();
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

CMaterial* CAssetManager::CreateMaterial(const string& key)
{
	CMaterial* material = GetMaterial(key);

	if (material == nullptr)
	{
		material = new CMaterial();
		m_materials.emplace(key, material);
	}

	material = CreateMaterialInstance(key);

	return material;
}

CMaterial* CAssetManager::CreateMaterialInstance(const string& key)
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

CMaterial* CAssetManager::GetMaterial(const string& key)
{
	CMaterial* material = nullptr;

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
	// ������Ʈ ������ ����� �ǿ��� ���� ���丮�� �����ϸ� Visual Studio���� ���� ��, �ش� ��θ� �۾� �����丮�� �����Ѵ�.
	// ������, Debug�� ����� ������ ���� �����ϴ� ��쿡��, �ش� ���� ������ ��ΰ� �۾� ���丮�� �����ǹǷ�, ���� �۾� ���丮����
	// ���� ������ �� �� ���� ��, Release\\Asset\\���� �̵��Ͽ� ���ҽ��� ������ �� �ֵ��� �����.
	// ��, � ���� �����ϴ��� �۾� ���丮�� ��ġ�ϵ��� ����� �ش�.
	char assetPath[255] = {};

	GetCurrentDirectoryA(255, assetPath);

	// ���� ���� ��θ� ���Ѵ�.
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

	LoadMeshes("Meshes.bin");
	LoadTextures("Textures.bin");
	LoadShaders();
	LoadMaterials("Materials.bin");
}

void CAssetManager::CreateShaderResourceViews()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	D3D12_CPU_DESCRIPTOR_HANDLE d3d12CpuDescriptorHandle = CGameFramework::GetInstance()->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE d3d12GpuDescriptorHandle = CGameFramework::GetInstance()->GetCbvSrvUavDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	UINT descriptorIncrementSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (const auto& texture : m_textures)
	{
		d3d12Device->CreateShaderResourceView(texture.second->GetTexture(), nullptr, d3d12CpuDescriptorHandle);
		texture.second->SetGpuDescriptorHandle(d3d12GpuDescriptorHandle);

		d3d12CpuDescriptorHandle.ptr += descriptorIncrementSize;
		d3d12GpuDescriptorHandle.ptr += descriptorIncrementSize;
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
