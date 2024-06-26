#pragma once
#include "TerrainObject.h"

class CObject;

class CScene abstract
{
	friend class CSceneManager;

public:
	string			 m_name;
	vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];

protected:
	// �� ��ü�� ������ ������ CSceneManager�� ���ؼ��� �Ͼ��.
	// ��, �� ��ü�� ��� ���� �ڽ� Ŭ������ �����ڿ��� �� Ŭ������ �����ڸ� ȣ���ؾ��ϹǷ� ���� �����ڸ� protected�� �����Ͽ���.
	CScene();

	void Load(const string& fileName);

private:
	virtual void Enter() = 0;
	virtual void Exit() = 0;

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	// �Ҹ����� ��쿡�� SafeDelete �ܺ� �Լ��� �̿��ϱ� ������ ���� �����ڸ� public���� �����Ͽ���.
	virtual ~CScene();

	void SetName(const string& name);
	const string& GetName();

	void AddObject(const GROUP_TYPE& groupType, CObject* object);

	const vector<CObject*>& GetGroupObject(GROUP_TYPE groupType);
	void DeleteGroupObject(GROUP_TYPE groupType);

	CTerrainObject* m_terrain = nullptr;
	float GetTerrainHeight(float x, float z) { 
		if (m_terrain && x >= 0 && z >= 0)
			return m_terrain->OnGetHeight(x, z);
		else
		{
			cout << "�ͷ��� ���� �ȵ�" << endl;
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
};
