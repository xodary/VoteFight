#pragma once

class CObject;

class CScene abstract
{
	friend class CSceneManager;

private:
	string			 m_name;

	vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];

protected:
	// �� ��ü�� ������ ������ CSceneManager�� ���ؼ��� �Ͼ��.
	// ��, �� ��ü�� ��� ���� �ڽ� Ŭ������ �����ڿ��� �� Ŭ������ �����ڸ� ȣ���ؾ��ϹǷ� ���� �����ڸ� protected�� �����Ͽ���.
	CScene();

	void Load(const string& fileName);
	void LoadUI(const string& fileName);

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

	void AddObject(GROUP_TYPE groupType, CObject* object);
	const vector<CObject*>& GetGroupObject(GROUP_TYPE groupType);
	void DeleteGroupObject(GROUP_TYPE groupType);

	virtual void Init() = 0;

	void ReleaseUploadBuffers();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();
};
