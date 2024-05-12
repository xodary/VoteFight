#include "pch.h"
#include "RigidBody.h"

#include "TimeManager.h"
#include "AssetManager.h"

#include "Object.h"

#include "Transform.h"

CRigidBody::CRigidBody() :
    m_mass(1.0f),
    m_force(),
    m_velocity(),
    m_accel(),
    m_maxSpeedXZ(),
    m_maxSpeedY()
{
}

CRigidBody::~CRigidBody()
{
}

void CRigidBody::SetMass(float mass)
{
    if (mass <= 0.0f)
    {
        mass = 1.0f;
    }

    m_mass = mass;
}

float CRigidBody::GetMass()
{
    return m_mass;
}

const XMFLOAT3& CRigidBody::GetVelocity()
{
    return m_velocity;
}

void CRigidBody::SetMaxSpeedXZ(float maxSpeedXZ)
{
    if (maxSpeedXZ < 0.0f)
    {
        maxSpeedXZ = 0.0f;
    }

    m_maxSpeedXZ = maxSpeedXZ;
}

float CRigidBody::GetMaxSpeedXZ()
{
    return m_maxSpeedXZ;
}

void CRigidBody::SetMaxSpeedY(float maxSpeedY)
{
    if (maxSpeedY < 0.0f)
    {
        maxSpeedY = 0.0f;
    }

    m_maxSpeedY = maxSpeedY;
}

float CRigidBody::GetMaxSpeedY()
{
    return m_maxSpeedY;
}

float CRigidBody::GetSpeedXZ()
{
    return Vector3::Length(XMFLOAT3(m_velocity.x, 0.0f, m_velocity.z));
}

float CRigidBody::GetSpeedY()
{
    return abs(m_velocity.y);
}

void CRigidBody::AddForce(const XMFLOAT3& force)
{
    m_force = Vector3::Add(m_force, force);
}

void CRigidBody::AddVelocity(const XMFLOAT3& velocity)
{
    m_velocity = Vector3::Add(m_velocity, velocity);
}

void CRigidBody::MovePosition()
{
    CTransform* transform = static_cast<CTransform*>(m_owner->GetComponent(COMPONENT_TYPE::TRANSFORM));
    const XMFLOAT3& position = transform->GetPosition();
    XMFLOAT3 shift = Vector3::ScalarProduct(m_velocity, DT);
    XMFLOAT3 newPosition = Vector3::Add(position, shift);

    transform->SetPosition(newPosition);
}

void CRigidBody::Update()
{
    if (!m_isEnabled)
    {
        return;
    }

    // 이번 프레임에 누적된 힘의 양에 따른 가속도 값 갱신
    // Force = Mass * Accel
    // Accel = Force / Mass = Force * (1.0f / Mass)
    m_accel = Vector3::ScalarProduct(m_force, 1.0f / m_mass);

    // 갱신된 가속도 값에 의한 속도 값 갱신
    m_velocity = Vector3::Add(m_velocity, Vector3::ScalarProduct(m_accel, DT));

    // XZ축 성분 처리
    float speedXZ = GetSpeedXZ();

    if (speedXZ > 0.0f)
    {
        // 이번 프레임에 X, Z축 성분으로 각각 누적된 힘이 없을 경우, 해당 성분에 매우 큰 마찰력을 적용하여 캐릭터가 빠르게 멈추게 만든다.
        float frictionCoeffX = (Math::IsZero(m_force.x)) ? 100.0f : 0.0f;
        float frictionCoeffZ = (Math::IsZero(m_force.z)) ? 100.0f : 0.0f;
        XMFLOAT3 direction = XMFLOAT3(m_velocity.x / speedXZ, 0.0f, m_velocity.z / speedXZ);
        XMFLOAT3 friction = Vector3::Inverse(direction);

        friction.x *= frictionCoeffX * DT;
        friction.z *= frictionCoeffZ * DT;

        // X축 마찰력이 현재 X축 속력보다 큰 경우
        if (abs(friction.x) >= abs(m_velocity.x))
        {
            m_velocity.x = 0.0f;
        }
        else
        {
            m_velocity.x += friction.x;
        }

        // Z축 마찰력이 현재 Z축 속력보다 큰 경우
        if (abs(friction.z) >= abs(m_velocity.z))
        {
            m_velocity.z = 0.0f;
        }
        else
        {
            m_velocity.z += friction.z;
        }

        // 마찰력을 적용한 이후의 속력을 다시 구한다.
        speedXZ = GetSpeedXZ();

        float maxSpeedXZ = m_maxSpeedXZ * DT;

        // 최대 속력 제한
        if (speedXZ > maxSpeedXZ)
        {
            // 초과 비율만큼 보정한다.
            float ratio = maxSpeedXZ / speedXZ;

            m_velocity.x *= ratio;
            m_velocity.z *= ratio;
        }

        MovePosition();
    }

    // 이번 프레임에 누적된 힘의 양 초기화
    m_force = XMFLOAT3(0.0f, 0.0f, 0.0f);
}
