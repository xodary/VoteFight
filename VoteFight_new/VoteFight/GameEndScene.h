#pragma once
#include "Scene.h"
#include "Player.h"

struct CB_GameEndScene
{
	LIGHT m_lights[MAX_LIGHTS];
	XMFLOAT4 gcGlobalAmbientLight = XMFLOAT4(0, 1, 1, 1);
	Fog   m_fog;
	int gnLights = 1;
};

class CGameEndScene : public CScene
{
	friend class CSceneManager;

private:
	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_GameEndScene* m_mappedGameScene;
public:
	static CGameEndScene* m_CGameEndScene;
	int m_rank[3];
private:
	CGameEndScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CGameEndScene();

	virtual void Init();

	void InitLight();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();

	virtual void RenderImGui();
	vector<CPlayer*> SortPlayersByTicket(const std::unordered_map<int, CObject*>& objects);
};
