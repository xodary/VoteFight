#pragma once
#include "TerrainObject.h"

class CObject;

struct Fog
{
	XMFLOAT4 m_color;
	float	 m_density;
};

struct LIGHT
{
	XMFLOAT4 m_xmf4Ambient;
	XMFLOAT4 m_xmf4Diffuse;
	XMFLOAT4 m_xmf4Specular;
	XMFLOAT4 m_xmf3Position;

	bool	   m_isActive;

	XMFLOAT3   m_position;
	XMFLOAT3   m_direction;

	int		   m_type;

	XMFLOAT3   m_attenuation;
	float 	   m_fallOff;
	float	   m_range;
	float 	   m_theta;
	float	   m_phi;

	bool	   m_shadowMapping;
	XMFLOAT4X4 m_toTexCoord;
};
class CScene abstract
{
	friend class CSceneManager;

public:
	string			 m_name;
	vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];	// 오브젝트들
	unordered_map<int, CObject*>	m_objects_id;	// ID가 있는 오브젝트들 
	unordered_set<CObject*> ObjectListSector[SECTOR_RANGE_ROW * SECTOR_RANGE_COL];	// 시야처리
	int oldXCell = -1, oldZCell = -1;
	unordered_set<CObject*>		my_vl;

protected:
	CScene();

	void Load(const string& fileName);

private:
	virtual void Enter() = 0;
	virtual void Exit() = 0;

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CScene();

	void SetName(const string& name);
	const string& GetName();

	void AddObject(const GROUP_TYPE& groupType, CObject* object);
	void AddObjectID(CObject* object, UINT id);

	const vector<CObject*>& GetGroupObject(GROUP_TYPE groupType);
	CObject* GetIDObject(int id);
	void DeleteGroupObject(GROUP_TYPE groupType);
	void DeleteObject(GROUP_TYPE groupType, CObject* object);

	CTerrainObject* m_terrain = nullptr;
	float GetTerrainHeight(float x, float z) { 
		if (m_terrain && x >= 0 && z >= 0)
			return m_terrain->OnGetHeight(x, z);
		else
		{
			cout << "Terrain not exist" << endl;
			return		0.f;
		}
	};
	CTerrainObject* GetTerrain() const { return m_terrain; };

	virtual void Init() = 0;

	void LoadUI(const string& fileName);
	void ReleaseUploadBuffers();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();

	virtual void RenderImGui();

	unordered_set<CObject*> GetViewList(int stateNum);

};
