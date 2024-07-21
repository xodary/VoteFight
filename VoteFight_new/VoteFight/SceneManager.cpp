#include "pch.h"
#include "SceneManager.h"
#include "GameFramework.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "SelectScene.h"
#include "LoginScene.h"

CSceneManager::CSceneManager() :
	m_scenes(),
	m_currentScene()
{
}

CSceneManager::~CSceneManager()
{
	Utility::SafeDelete(m_scenes);
}

void CSceneManager::ChangeScene(SCENE_TYPE sceneType)
{
	m_currentScene->Exit();

	m_sceneType = sceneType;
	//m_scenes[static_cast<int>(SCENE_TYPE(sceneType))]->Init();
	m_currentScene = m_scenes[static_cast<int>(sceneType)];
	m_currentScene->Enter();
}

CScene* CSceneManager::GetCurrentScene()
{
	return m_currentScene;
}

CGameScene* CSceneManager::GetGameScene()
{
	return static_cast<CGameScene*>(m_scenes[static_cast<int>(SCENE_TYPE::GAME)]);
}

void CSceneManager::Init()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	// �� ����
	m_scenes.resize(static_cast<int>(SCENE_TYPE::COUNT));
	m_scenes[static_cast<int>(SCENE_TYPE::SELECT)] = new CSelectScene();
	m_scenes[static_cast<int>(SCENE_TYPE::LOGIN)] = new CLoginScene();
	m_scenes[static_cast<int>(SCENE_TYPE::GAME)] = new CGameScene();

	m_scenes[static_cast<int>(SCENE_TYPE::GAME)]->Init();
	m_scenes[static_cast<int>(SCENE_TYPE::SELECT)]->Init();
	m_scenes[static_cast<int>(SCENE_TYPE::LOGIN)]->Init();

	m_sceneType = SCENE_TYPE::LOGIN;

	m_currentScene = m_scenes[static_cast<int>(SCENE_TYPE::LOGIN)];
	m_currentScene->Enter();
}

void CSceneManager::ReleaseUploadBuffers()
{
	for (int i = 0; i < static_cast<int>(SCENE_TYPE::COUNT); ++i)
	{
		if (m_scenes[i] != nullptr)
		{
			m_scenes[i]->ReleaseUploadBuffers();
		}
	}
}

void CSceneManager::Update()
{
	m_currentScene->Update();
}

void CSceneManager::PreRender()
{
	m_currentScene->PreRender();
}

void CSceneManager::Render()
{
	m_currentScene->Render();
}

void CSceneManager::PostRender()
{
	m_currentScene->PostRender();
}

