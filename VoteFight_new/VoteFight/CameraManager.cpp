#include "pch.h"
#include "CameraManager.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "Transform.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Player.h"

CCameraManager::CCameraManager() :
	m_cameras()
{
}

CCameraManager::~CCameraManager()
{
}

CCamera* CCameraManager::GetMainCamera()
{
	return m_cameras[0];
}

CCamera* CCameraManager::GetUICamera()
{
	return m_cameras[1];
}

const vector<CCamera*>& CCameraManager::GetCameras()
{
	return m_cameras;
}

void CCameraManager::Init()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();
	CCamera* camera = nullptr;

	camera = new CCamera(CAMERA_TYPE::MAIN);
	camera->SetViewport(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y), 0.0f, 1.0f);
	camera->SetScissorRect(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y));
	camera->SetOffset(XMFLOAT3(6, 10, -6));
	camera->SetSpeed(12.0f);
	camera->GeneratePerspectiveProjectionMatrix(90.0f, resolution.x / resolution.y, 1.0f, 500.0f);
	camera->GenerateViewMatrix(XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-0.4f, -0.1f, 0.4f));
	camera->CreateShaderVariables();
	m_cameras.push_back(camera);

	camera = new CCamera(CAMERA_TYPE::UI);
	camera->SetViewport(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y), 0.0f, 1.0f);
	camera->SetScissorRect(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y));
	camera->GenerateOrthographicsProjectionMatrix(resolution.x, resolution.y, 0.0f, 30.0f);
	camera->GenerateViewMatrix(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));
	camera->CreateShaderVariables();
	m_cameras.push_back(camera);

	camera = new CCamera(CAMERA_TYPE::LIGHT);
	camera->SetViewport(0, 0, DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT, 0.0f, 1.0f);
	camera->SetScissorRect(0, 0, DEPTH_BUFFER_WIDTH, DEPTH_BUFFER_HEIGHT);
	camera->CreateShaderVariables();
	m_cameras.push_back(camera);
}

void CCameraManager::SetSelectSceneMainCamera()
{
	const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();

	CCamera* camera = GetMainCamera();
	camera->SetViewport(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y), 0.0f, 1.0f);
	camera->SetScissorRect(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y));
	camera->GeneratePerspectiveProjectionMatrix(30.0f, resolution.x / resolution.y, 1.0f, 500.0f);
	camera->GenerateViewMatrix(XMFLOAT3(4.0f, 3.0f, -13.f), XMFLOAT3(0.0f, 0.0f, 1.0f));
	camera->CreateShaderVariables();
}

void CCameraManager::SetGameSceneMainCamera()
{
	const XMFLOAT2& resolution = CGameFramework::GetInstance()->GetResolution();

	CCamera* camera = GetMainCamera();
	camera->SetViewport(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y), 0.0f, 1.0f);
	camera->SetScissorRect(0, 0, static_cast<UINT>(resolution.x), static_cast<UINT>(resolution.y));
	camera->SetOffset(XMFLOAT3(3.0f, 6.f, -3.f));
	camera->SetSpeed(12.0f);
	camera->GeneratePerspectiveProjectionMatrix(90.0f, resolution.x / resolution.y, 1.0f, 500.0f);
	camera->GenerateViewMatrix(XMFLOAT3(10.0f, 1.0f, -10.0f), XMFLOAT3(-0.4f, -0.1f, 0.4f));
	camera->CreateShaderVariables();
}

void CCameraManager::Update()
{
	GetMainCamera()->Update();
}

