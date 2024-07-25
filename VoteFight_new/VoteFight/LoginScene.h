#pragma once
#include "Scene.h"

class CLoginScene : public CScene
{
public:
	friend class CSceneManager;

	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_GameScene* m_mappedGameScene;

	CObject* m_focus;

	XMFLOAT3	m_fcamera[4] = { XMFLOAT3(50, 10, 50), XMFLOAT3(75, 60, 335), XMFLOAT3(366, 10, 344), XMFLOAT3(366, 10, 50) };
	float		time = 1;
	int			m_ncamera = 0;

	char login_state[255] = "Enter User Name";
	char server_addr[255] = "127.0.0.1";
	char user_name[255] = "";

	int button_opacity = 255;
	bool active = true;

	CLoginScene();
	virtual ~CLoginScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void Init();
	virtual void InitUI();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
	virtual void RenderImGui();

	bool startSong = false;
};