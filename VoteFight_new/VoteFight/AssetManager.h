#pragma once

class CObject;
class CMesh;
class CSkinnedMesh;
class CTexture;
class CShader;
class CMaterial;
class CAnimation;

class CAssetManager : public CSingleton<CAssetManager>
{
	friend class CSingleton;

private:
	string							           m_assetPath;

	unordered_map<string, CMesh*>              m_meshes;
	unordered_map<string, CTexture*>           m_textures;
	unordered_map<string, CShader*>            m_shaders;
	unordered_map<string, CMaterial*>          m_materials;
	unordered_map<string, vector<CAnimation*>> m_animations; // [modelName][animation]

private:
	CAssetManager();
	virtual ~CAssetManager();

	void LoadMeshes(const string& fileName);
	void LoadTextures(const string& fileName);
	void LoadShaders();
	void LoadMaterials(const string& fileName);

public:
	const string& GetAssetPath();

	CMesh* GetMesh(string& key);
	int GetMeshCount();

	CTexture* CreateTexture(const string& key, const string& textureName, const TEXTURE_TYPE& type);
	CTexture* GetTexture(const string& key);
	int GetTextureCount();

	CShader* GetShader(const string& key);
	int GetShaderCount();

	CMaterial* CreateMaterial(string& key);
	CMaterial* CreateMaterialInstance(string& key);
	CMaterial* GetMaterial(string& key);
	int GetMaterialCount();

	void LoadSkinningAnimations(const string& fileName);
	void LoadUIAnimations(ifstream& in, const string& key);
	const vector<CAnimation*>& GetAnimations(const string& key);
	int GetAnimationCount(const string& key);

	virtual void Init();

	void CreateShaderResourceViews();

	void ReleaseUploadBuffers();
};
