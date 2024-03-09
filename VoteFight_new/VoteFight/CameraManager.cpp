#include "pch.h"
#include "CameraManager.h"

#include "GameFramework.h"

#include "TimeManager.h"
#include "InputManager.h"

#include "Camera.h"

#include "Transform.h"

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
	camera->SetOffset(XMFLOAT3(0.0f, 4.5f, -3.5f));
	camera->SetSpeed(12.0f);
	camera->GeneratePerspectiveProjectionMatrix(90.0f, resolution.x / resolution.y, 1.0f, 200.0f);
	camera->GenerateViewMatrix(XMFLOAT3(0.0f, 5.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));
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

void CCameraManager::Update()
{
	// 현재 윈도우가 포커싱 상태인지 알아낸다.
	//HWND hWnd = GetFocus();

	//if (hWnd != nullptr)
	//{
	//	// 윈도우 영역 계산
	//	RECT rect = {};

	//	GetWindowRect(hWnd, &rect);

	//	// 마우스 커서 위치 계산
	//	POINT oldCursor = { static_cast<LONG>(rect.right / 2), static_cast<LONG>(rect.bottom / 2) };
	//	POINT cursor = {};

	//	// 이 함수는 윈도우 전체 영역을 기준으로 커서의 위치를 계산한다.
	//	GetCursorPos(&cursor);

	//	XMFLOAT2 delta = {};

	//	delta.x = (cursor.x - oldCursor.x) * 20.0f * DT;
	//	delta.y = (cursor.y - oldCursor.y) * 20.0f * DT;

	//	SetCursorPos(oldCursor.x, oldCursor.y);

	//	CTransform* transform = GetMainCamera()->GetComponent<CTransform>();

	//	transform->Rotate(XMFLOAT3(delta.y, delta.x, 0.0f));

	//	XMFLOAT3 shift = {};

	//	if (KEY_HOLD(KEY::UP))
	//	{
	//		shift = Vector3::Add(shift, Vector3::ScalarProduct(transform->GetForward(), 40.0f * DT));
	//	}

	//	if (KEY_HOLD(KEY::DOWN))
	//	{
	//		shift = Vector3::Add(shift, Vector3::ScalarProduct(transform->GetForward(), -40.0f * DT));
	//	}

	//	if (KEY_HOLD(KEY::LEFT))
	//	{
	//		shift = Vector3::Add(shift, Vector3::ScalarProduct(transform->GetRight(), -40.0f * DT));
	//	}

	//	if (KEY_HOLD(KEY::RIGHT))
	//	{
	//		shift = Vector3::Add(shift, Vector3::ScalarProduct(transform->GetRight(), 40.0f * DT));
	//	}

	//	transform->Translate(shift);
	//}

	GetMainCamera()->Update();
}
