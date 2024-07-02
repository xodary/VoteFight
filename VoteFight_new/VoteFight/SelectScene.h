#pragma once
#include "Scene.h"
#include "Player.h"

struct CB_SelectScene
{
	LIGHT m_lights[MAX_LIGHTS];
	XMFLOAT4 gcGlobalAmbientLight = XMFLOAT4(0, 1, 1, 1);
	Fog   m_fog;
	int gnLights = 1;
};

class CSelectScene : public CScene
{
	friend class CSceneManager;

private:
	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_SelectScene* m_mappedGameScene;

	CObject* m_SelectCharacter;
	CObject* m_WaitCharacters[3];
public:
	static CSelectScene* m_CSelectScene;

private:
	CSelectScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CSelectScene();

	virtual void Init();

	void InitLight();
	void SelectCharacter(UINT i);

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
};
