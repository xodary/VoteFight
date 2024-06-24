#pragma once
#include "Scene.h"

class CLoginScene : public CScene
{
public:
	friend class CSceneManager;

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
};