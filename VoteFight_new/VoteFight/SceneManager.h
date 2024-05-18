#pragma once

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
	CScene* GetCurrentScene();
	void ChangeScene(SCENE_TYPE sceneType);


	virtual void Init();

	void ReleaseUploadBuffers();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

	//void ProcessPacket();
};
