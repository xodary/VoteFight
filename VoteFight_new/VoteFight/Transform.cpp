#include "pch.h"
#include "Transform.h"

#include "GameFramework.h"

#include "Object.h"

const XMFLOAT3 CTransform::m_worldRight   = XMFLOAT3(1.0f, 0.0f, 0.0f);
const XMFLOAT3 CTransform::m_worldUp      = XMFLOAT3(0.0f, 1.0f, 0.0f);
const XMFLOAT3 CTransform::m_worldForward = XMFLOAT3(0.0f, 0.0f, 1.0f);

CTransform::CTransform() :
	m_position(),
	m_rotation(),
	m_scale(XMFLOAT3(1.0f, 1.0f, 1.0f)),
	m_localPosition(),
	m_localRotation(),
	m_localScale(XMFLOAT3(1.0f, 1.0f, 1.0f)),
	m_worldMatrix(Matrix4x4::Identity())
{
}

CTransform::~CTransform()
{
}

void CTransform::SetPosition(const XMFLOAT3& position)
{
	// 부모가 없는 최상위 노드가 아니었다면, localPosition을 조정하여 position이 되도록 설정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localPosition = Vector3::Add(m_localPosition, Vector3::Subtract(position, m_position));
	}
	else
	{
		m_position = position;
	}
}

const XMFLOAT3& CTransform::GetPosition()
{
	return m_position;
}

void CTransform::SetRotation(const XMFLOAT3& rotation)
{
	// 부모가 없는 최상위 노드가 아니었다면, localRotation을 조정하여 rotation이 되도록 설정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localRotation = Vector3::Add(m_localRotation, Vector3::Subtract(rotation, m_rotation));
	}
	else
	{
		m_rotation = rotation;
	}
}

const XMFLOAT3& CTransform::GetRotation()
{
	return m_rotation;
}

void CTransform::SetScale(const XMFLOAT3& scale)
{
	// 부모가 없는 최상위 노드가 아니었다면, localScale을 조정하여 scale이 되도록 설정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localScale = Vector3::Add(m_localScale, Vector3::Subtract(scale, m_scale));
	}
	else
	{
		m_scale = scale;
	}
}

const XMFLOAT3& CTransform::GetScale()
{
	return m_scale;
}

void CTransform::SetLocalPosition(const XMFLOAT3& localPosition)
{
	m_localPosition = localPosition;
}

const XMFLOAT3& CTransform::GetLocalPosition()
{
	return m_localPosition;
}

void CTransform::SetLocalRotation(const XMFLOAT3& localRotation)
{
	m_localRotation = localRotation;
}

const XMFLOAT3& CTransform::GetLocalRotation()
{
	return m_localRotation;
}

void  CTransform::SetLocalScale(const XMFLOAT3& localScale)
{
	m_localScale = localScale;
}

const XMFLOAT3& CTransform::GetLocalScale()
{
	return m_localScale;
}

const XMFLOAT4X4& CTransform::GetWorldMatrix()
{
	return m_worldMatrix;
}

XMFLOAT3 CTransform::GetRight()
{
	XMFLOAT3 right = Vector3::TransformNormal(m_worldRight, Matrix4x4::Rotation(m_rotation));
	CObject* parent = m_owner->GetParent();

	// 부모가 없는 최상위 노드가 아니었다면, 부모 행렬을 곱한다.
	if (parent != nullptr)
	{
		CTransform* parentTransform = static_cast<CTransform*>(parent->GetComponent(COMPONENT_TYPE::TRANSFORM));

		right = Vector3::TransformNormal(right, parentTransform->GetWorldMatrix());
	}

	return right;
}

XMFLOAT3 CTransform::GetUp()
{
	XMFLOAT3 up = Vector3::TransformNormal(m_worldUp, Matrix4x4::Rotation(m_rotation));
	CObject* parent = m_owner->GetParent();

	// 부모가 없는 최상위 노드가 아니었다면, 부모 행렬을 곱한다.
	if (parent != nullptr)
	{
		CTransform* parentTransform = static_cast<CTransform*>(parent->GetComponent(COMPONENT_TYPE::TRANSFORM));

		up = Vector3::TransformNormal(up, parentTransform->GetWorldMatrix());
	}

	return up;
}

XMFLOAT3 CTransform::GetForward()
{
	XMFLOAT3 forward = Vector3::TransformNormal(m_worldForward, Matrix4x4::Rotation(m_rotation));
	CObject* parent = m_owner->GetParent();

	// 부모가 없는 최상위 노드가 아니었다면, 부모 행렬을 곱한다.
	if (parent != nullptr)
	{
		CTransform* parentTransform = static_cast<CTransform*>(parent->GetComponent(COMPONENT_TYPE::TRANSFORM));

		forward = Vector3::TransformNormal(forward, parentTransform->GetWorldMatrix());
	}

	return forward;
}

void CTransform::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	XMFLOAT4X4 worldMatrix = {};

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &worldMatrix, 0);
}

void CTransform::Translate(const XMFLOAT3& shift)
{
	// 부모가 없는 최상위 노드가 아니었다면, localPosition을 조정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localPosition = Vector3::Add(m_localPosition, shift);
	}
	else
	{
		m_position = Vector3::Add(m_position, shift);
	}
}

void CTransform::Rotate(const XMFLOAT3& rotation)
{
	// 부모가 없는 최상위 노드가 아니었다면, localRotation을 조정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localRotation = Vector3::Add(m_localRotation, rotation);
	}
	else
	{
		m_rotation = Vector3::Add(m_rotation, rotation);
	}
}

void CTransform::LookTo(const XMFLOAT3& direction)
{
	// 회전 행렬로부터 쿼터니언을 얻어온다.
	// 이때, DirectXMath의 LookToLH 함수는 카메라 변환 행렬을 구하기 위한 함수이기 때문에 역행렬을 구해야 한다.
	// 하지만 회전 행렬은 직교 행렬이므로 전치 행렬과 역행렬이 같으므로 연산이 더 적은 전치 행렬을 사용하였다.
	XMFLOAT4X4 rotationMatrix = Matrix4x4::Transpose(Matrix4x4::LookToLH(XMFLOAT3(0.0f, 0.0f, 0.0f), direction, m_worldUp));
	XMFLOAT4 quaternion = {};

	XMStoreFloat4(&quaternion, XMQuaternionRotationMatrix(XMLoadFloat4x4(&rotationMatrix)));

	// forward가 direction이 되도록 회전하기 위한 오일러 각을 구한다.
	// 쿼터니언에서 오일러각을 추출한다.
	// 참고: https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=spinx85&logNo=140120555548 
	float qx = quaternion.x, qy = quaternion.y, qz = quaternion.z, qw = quaternion.w;
	float sqx = qx * qx, sqy = qy * qy, sqz = qz * qz, sqw = qw * qw;
	XMFLOAT3 rotation = {};

	rotation.x = XMConvertToDegrees(asinf(2.0f * (qx * qw - qy * qz)));
	rotation.y = XMConvertToDegrees(atan2f(2.0f * (qx * qz + qy * qw), -sqx - sqy + sqz + sqw));
	rotation.z = XMConvertToDegrees(atan2f(2.0f * (qx * qy + qz * qw), -sqx + sqy - sqz + sqw));
	SetRotation(rotation);
}

void CTransform::AddScale(const XMFLOAT3& scale)
{
	// 부모가 없는 최상위 노드가 아니었다면, localScale을 조정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localScale = Vector3::Add(m_localScale, scale);
	}
	else
	{
		m_scale = Vector3::Add(m_scale, scale);
	}
}

void CTransform::MultiplyScale(const XMFLOAT3& scale)
{
	// 부모가 없는 최상위 노드가 아니었다면, localScale을 조정한다.
	if (m_owner->GetParent() != nullptr)
	{
		m_localScale = Vector3::Multiply(m_localScale, scale);
	}
	else
	{
		m_scale = Vector3::Multiply(m_scale, scale);
	}
}

void CTransform::Update()
{
	// W = S * R * T
	// 월드 변환 행렬은 매프레임 변화한 이동, 회전, 신축정보를 반영하여 새로 계산한다.
	m_worldMatrix = Matrix4x4::Identity();

	CObject* parent = m_owner->GetParent();

	if (parent != nullptr)
	{
		// scale 값을 이용하여 신축 행렬을 구한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Scale(m_localScale));

		// roation 값을 이용하여 회전 행렬을 구한다.
		// 이때, 짐벌락 문제가 발생하지 않도록 사원수를 사용하는 DirectX의 API를 이용한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Rotation(m_localRotation));

		// position 값을 이용하여 이동 행렬을 구한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Translation(m_localPosition));

		// 위에서 구한 월드 변환 행렬에 부모의 월드 변환행렬을 곱한다.
		CTransform* parentTransform = static_cast<CTransform*>(parent->GetComponent(COMPONENT_TYPE::TRANSFORM));

		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, parentTransform->GetWorldMatrix());
		m_position = Vector3::Add(parentTransform->GetPosition(), m_localPosition);
		m_rotation = Vector3::Add(parentTransform->GetRotation(), m_localRotation);
		m_scale = Vector3::Multiply(parentTransform->GetScale(), m_localScale);
	}
	else
	{
		// scale 값을 이용하여 신축 행렬을 구한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Scale(m_scale));

		// roation 값을 이용하여 회전 행렬을 구한다.
		// 이때, 짐벌락 문제가 발생하지 않도록 사원수를 사용하는 DirectX의 API를 이용한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Rotation(m_rotation));

		// position 값을 이용하여 이동 행렬을 구한다.
		m_worldMatrix = Matrix4x4::Multiply(m_worldMatrix, Matrix4x4::Translation(m_position));
	}
}

//=========================================================================================================================

CRectTransform::CRectTransform() :
	m_rect()
{
}

CRectTransform::~CRectTransform()
{
}

void CRectTransform::SetRect(const XMFLOAT2& rect)
{
	if ((rect.x >= 0.0f) && (rect.y >= 0.0f))
	{
		m_rect = rect;
	}
}

const XMFLOAT2& CRectTransform::GetRect()
{
	return m_rect;
}

void CRectTransform::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	XMFLOAT4X4 worldMatrix = Matrix4x4::Multiply(Matrix4x4::Scale(XMFLOAT3(m_rect.x, m_rect.y, 1.0f)), m_worldMatrix);

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
	d3d12GraphicsCommandList->SetGraphicsRoot32BitConstants(static_cast<UINT>(ROOT_PARAMETER_TYPE::OBJECT), 16, &worldMatrix, 0);
}
