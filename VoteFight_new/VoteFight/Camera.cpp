#include "pch.h"
#include "Camera.h"

#include "GameFramework.h"

#include "TimeManager.h"

#include "Transform.h"

CCamera::CCamera(CAMERA_TYPE type) :
	m_type(type),
	m_isZoomIn(),
	m_magnification(1.0f),
	m_d3d12Viewport(),
	m_d3d12ScissorRect(),
	m_viewMatrix(Matrix4x4::Identity()),
	m_projectionMatrix(Matrix4x4::Identity()),
	m_offset(),
	m_speed(),
	m_frustum(),
	m_d3d12Buffer(),
	m_mappedData(),
	m_target(),
	m_light()
{
}

CCamera::~CCamera()
{
	ReleaseShaderVariables();
}

CAMERA_TYPE CCamera::GetType()
{
	return m_type;
}

void CCamera::SetZoomIn(bool isZoomIn)
{
	m_isZoomIn = isZoomIn;
}

bool CCamera::IsZoomIn()
{
	return m_isZoomIn;
}

void CCamera::SetMagnification(float magnification)
{
	m_magnification = clamp(magnification, 1.0f, 1.5f);
}

float CCamera::GetMagnification()
{
	return m_magnification;
}

void CCamera::SetViewport(int topLeftX, int topLeftY, UINT width, UINT height, float minDepth, float maxDepth)
{
	m_d3d12Viewport.TopLeftX = static_cast<float>(topLeftX);
	m_d3d12Viewport.TopLeftY = static_cast<float>(topLeftY);
	m_d3d12Viewport.Width = static_cast<float>(width);
	m_d3d12Viewport.Height = static_cast<float>(height);
	m_d3d12Viewport.MinDepth = minDepth;
	m_d3d12Viewport.MaxDepth = maxDepth;
}

void CCamera::SetScissorRect(LONG left, LONG top, LONG right, LONG bottom)
{
	m_d3d12ScissorRect.left = left;
	m_d3d12ScissorRect.top = top;
	m_d3d12ScissorRect.right = right;
	m_d3d12ScissorRect.bottom = bottom;
}

const XMFLOAT4X4& CCamera::GetViewMatrix()
{
	return m_viewMatrix;
}

const XMFLOAT4X4& CCamera::GetProjectionMatrix()
{
	return m_projectionMatrix;
}

void CCamera::SetOffset(const XMFLOAT3& offset)
{
	m_offset = offset;
}

const XMFLOAT3& CCamera::GetOffset()
{
	return m_offset;
}

void CCamera::SetSpeed(float speed)
{
	m_speed = max(0.0f, speed);
}

float CCamera::GetSpeed()
{
	return m_speed;
}

void CCamera::SetTarget(CObject* target)
{
	m_target = target;
}

CObject* CCamera::GetTarget()
{
	return m_target;
}

void CCamera::SetLight(LIGHT* light)
{
	m_light = light;
}

LIGHT* CCamera::GetLight()
{
	return m_light;
}

void CCamera::CreateShaderVariables()
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	UINT bytes = (sizeof(CB_CAMERA) + 255) & ~255;

	m_d3d12Buffer = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
	DX::ThrowIfFailed(m_d3d12Buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
}

void CCamera::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	XMStoreFloat4x4(&m_mappedData->m_viewMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_viewMatrix)));
	XMStoreFloat4x4(&m_mappedData->m_projectionMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_projectionMatrix)));
	memcpy(&m_mappedData->m_position, &transform->GetPosition(), sizeof(XMFLOAT3));
	d3d12GraphicsCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(ROOT_PARAMETER_TYPE::CAMERA), m_d3d12Buffer->GetGPUVirtualAddress());
}

void CCamera::ReleaseShaderVariables()
{
	m_d3d12Buffer->Unmap(0, nullptr);
}

void CCamera::RSSetViewportsAndScissorRects()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	d3d12GraphicsCommandList->RSSetViewports(1, &m_d3d12Viewport);
	d3d12GraphicsCommandList->RSSetScissorRects(1, &m_d3d12ScissorRect);
}

void CCamera::GenerateViewMatrix(const XMFLOAT3& position, const XMFLOAT3& forward)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	transform->SetPosition(position);
	transform->LookTo(forward);

	RegenerateViewMatrix();
}

void CCamera::RegenerateViewMatrix()
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	m_viewMatrix = Matrix4x4::LookToLH(transform->GetPosition(), transform->GetForward(), CTransform::m_worldUp);

	GenerateBoundingFrustum();
}

void CCamera::GenerateOrthographicsProjectionMatrix(float ViewWidth, float ViewHeight, float NearZ, float FarZ)
{
	m_projectionMatrix = Matrix4x4::OrthographicFovLH(ViewWidth, ViewHeight, NearZ, FarZ);
}

void CCamera::GeneratePerspectiveProjectionMatrix(float FOVAngleY, float AspectRatio, float NearZ, float FarZ)
{
	m_projectionMatrix = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(FOVAngleY), AspectRatio, NearZ, FarZ);
}

void CCamera::GenerateBoundingFrustum()
{
	// 원근 투영 변환 행렬에서 절두체를 생성한다(절두체는 카메라 좌표계로 표현된다).
	m_frustum.CreateFromMatrix(m_frustum, XMLoadFloat4x4(&m_projectionMatrix));

	// 카메라 변환 행렬의 역행렬을 구한다.
	XMMATRIX inversedViewMatrix = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_viewMatrix));

	// 절두체를 카메라 변환 행렬의 역행렬로 변환한다(이제 절두체는 월드 좌표계로 표현된다).
	m_frustum.Transform(m_frustum, inversedViewMatrix);
}

bool CCamera::IsInBoundingFrustum(const BoundingBox& BoundingBox)
{
	return m_frustum.Intersects(BoundingBox);
}

void CCamera::Update()
{
	if (m_target != nullptr)
	{
		CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
		CTransform* targetTransform = static_cast<CTransform*>(m_target->GetComponent(COMPONENT_TYPE::TRANSFORM));
		XMFLOAT3 rotation = Vector3::Add(XMFLOAT3(transform->GetRotation().x, 0.0f, 0.0f), targetTransform->GetRotation());

		transform->SetRotation(rotation);

		XMFLOAT4X4 rotationMatrix = Matrix4x4::Rotation(rotation);
		XMFLOAT3 focusPosition = targetTransform->GetPosition();

		if (m_isZoomIn)
		{
			XMFLOAT3 zoomDirection = Vector3::Normalize(Vector3::Add(transform->GetRight(), transform->GetForward()));

			zoomDirection = Vector3::ScalarProduct(zoomDirection, m_magnification);
			focusPosition = Vector3::Add(focusPosition, zoomDirection);
		}

		XMFLOAT3 newPosition = Vector3::Add(focusPosition, Vector3::TransformNormal(m_offset, rotationMatrix));
		XMFLOAT3 direction = Vector3::Subtract(newPosition, transform->GetPosition());
		float length = m_speed * Vector3::Length(direction) * DT;

		if (length > 0.0f)
		{
			direction = Vector3::Normalize(direction);
			transform->Translate(Vector3::ScalarProduct(direction, length));
		}
	}

	RegenerateViewMatrix();
}
