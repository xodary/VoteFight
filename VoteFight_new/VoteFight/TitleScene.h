#pragma once
#include "Scene.h"
#include "Player.h"

struct CB_TitleScene
{
	LIGHT m_lights[MAX_LIGHTS];
	XMFLOAT4 gcGlobalAmbientLight = XMFLOAT4(0, 1, 1, 1);
	Fog   m_fog;
	int gnLights = 1;
};

class CTitleScene : public CScene
{
	friend class CSceneManager;

private:
	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_TitleScene* m_mappedGameScene;

	//INIT_GAME_DATA		   m_InitGameData{};

public:
	static CTitleScene* m_CTitleScene;

private:
	CTitleScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CTitleScene();

	virtual void Init();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
};
