#pragma once
#include "Scene.h"
#include "Player.h"

struct CB_GameScene
{
	LIGHT m_lights[MAX_LIGHTS];
	XMFLOAT4 gcGlobalAmbientLight = XMFLOAT4(0,1,1,1);
	Fog   m_fog;
	int gnLights = 1;
};

class CGameScene : public CScene
{
	friend class CSceneManager;

private:
	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_GameScene*		   m_mappedGameScene;

	CObject*			   m_towerLight;
	float				   m_towerLightAngle;
	bool					m_miniMap = false;
	//INIT_GAME_DATA		   m_InitGameData{};

public:
	bool					inven;
	static CGameScene*		m_CGameScene;
	vector<CPlayer*>		m_otherPlayers;
	vector<vector<float>>	m_heights;

	CGameScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual ~CGameScene();

	virtual void Init();

	void InitLight();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void RenderImGui();

	virtual void Sea_level_rise(float newPhaseHeight);

	bool is_OceanRise = false;
	float m_fOceanRiseTime = 0.f;
	float m_fOceanHeight = 0.f;
	CObject* m_Ocean = nullptr;

	// 미니맵 관련 함수
	void ImGuiRenderMiniMap();

	// 라이트 설정 함수
	void SetLightVersion(int value);
};
