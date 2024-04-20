#pragma once
#include "Terrain.h"

class CObject;

class CScene abstract
{
	friend class CSceneManager;

private:
	string			 m_name;
	vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];
	CTerrain* m_terrain = nullptr;

protected:
	// 이 객체의 생성은 오로지 CSceneManager에 의해서만 일어난다.
	// 단, 이 객체를 상속 받은 자식 클래스의 생성자에서 이 클래스의 생성자를 호출해야하므로 접근 지정자를 protected로 설정하였다.
	CScene();

	void Load(const string& fileName);

private:
	virtual void Enter() = 0;
	virtual void Exit() = 0;

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	// 소멸자의 경우에는 SafeDelete 외부 함수를 이용하기 때문에 접근 지정자를 public으로 설정하였다.
	virtual ~CScene();

	void SetName(const string& name);
	const string& GetName();

	void AddObject(const GROUP_TYPE& groupType, CObject* object);

	const vector<CObject*>& GetGroupObject(GROUP_TYPE groupType);
	void DeleteGroupObject(GROUP_TYPE groupType);

	void CreateTerrain() { m_terrain = new CTerrain(257,257); };
	float GetTerrainHeight(float x, float y) { 
		if (m_terrain)
			return m_terrain->OnGetHeight(x, y);
		else
		{
			cout << "터레인 생성 안됨" << endl;
			return		0.f;
		}
	};
	CTerrain* GetTerrain() const { return m_terrain; };

	virtual void Init() = 0;

	void LoadUI(const string& fileName);

	void ReleaseUploadBuffers();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();
};
