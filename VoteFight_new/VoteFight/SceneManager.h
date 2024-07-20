#pragma once
#include "GameScene.h"

class CScene;

class CSceneManager : public CSingleton<CSceneManager>
{
	friend class CSingleton<CSceneManager>;

private:
	vector<CScene*> m_scenes;
	CScene* m_currentScene;
private:
	CSceneManager();
	virtual ~CSceneManager();

public:
	SCENE_TYPE m_sceneType;

	void ChangeScene(SCENE_TYPE sceneType);
	CScene* GetCurrentScene();
	CGameScene* GetGameScene();


	virtual void Init();

	void ReleaseUploadBuffers();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

	//void ProcessPacket();
};
