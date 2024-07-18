#pragma once
#include "Scene.h"
#include "Player.h"
#include "AssetManager.h"


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

	int m_round = 1;
	//INIT_GAME_DATA		   m_InitGameData{};

public:
	static CGameScene*		m_CGameScene;
	vector<CPlayer*>		m_otherPlayers;
	vector<vector<float>>	m_heights;

private:
	CGameScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CGameScene();

	virtual void Init();

	void InitLight();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void RenderImGui();

	void LoadFonts(const string& fileName)
	{
		ImGuiIO& io = ImGui::GetIO();
		// 기본 폰트 크기 18.0f, 원하는 폰트 경로로 변경
		string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Font\\" + fileName + ".ttf";
		io.Fonts->AddFontFromFileTTF(filePath.data(), 18.0f);
		// 폰트가 다시 로드될 수 있도록 이 함수를 호출해야 합니다.
		io.Fonts->Build();
	}
};
