#include "pch.h"
#include "RigidBody.h"

#include "TimeManager.h"
#include "AssetManager.h"

#include "Object.h"

#include "NavMesh.h"

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
    CNavMesh* navMesh = static_cast<CNavMesh*>(CAssetManager::GetInstance()->GetMesh("NavMesh"));

    if (navMesh->IsInNavMesh(position, newPosition))
    {
        transform->SetPosition(newPosition);
    }
}

void CRigidBody::Update()
{
    if (!m_isEnabled)
    {
        return;
    }

    // �̹� �����ӿ� ������ ���� �翡 ���� ���ӵ� �� ����
    // Force = Mass * Accel
    // Accel = Force / Mass = Force * (1.0f / Mass)
    m_accel = Vector3::ScalarProduct(m_force, 1.0f / m_mass);

    // ���ŵ� ���ӵ� ���� ���� �ӵ� �� ����
    m_velocity = Vector3::Add(m_velocity, Vector3::ScalarProduct(m_accel, DT));

    // XZ�� ���� ó��
    float speedXZ = GetSpeedXZ();

    if (speedXZ > 0.0f)
    {
        // �̹� �����ӿ� X, Z�� �������� ���� ������ ���� ���� ���, �ش� ���п� �ſ� ū �������� �����Ͽ� ĳ���Ͱ� ������ ���߰� �����.
        float frictionCoeffX = (Math::IsZero(m_force.x)) ? 100.0f : 0.0f;
        float frictionCoeffZ = (Math::IsZero(m_force.z)) ? 100.0f : 0.0f;
        XMFLOAT3 direction = XMFLOAT3(m_velocity.x / speedXZ, 0.0f, m_velocity.z / speedXZ);
        XMFLOAT3 friction = Vector3::Inverse(direction);

        friction.x *= frictionCoeffX * DT;
        friction.z *= frictionCoeffZ * DT;

        // X�� �������� ���� X�� �ӷº��� ū ���
        if (abs(friction.x) >= abs(m_velocity.x))
        {
            m_velocity.x = 0.0f;
        }
        else
        {
            m_velocity.x += friction.x;
        }

        // Z�� �������� ���� Z�� �ӷº��� ū ���
        if (abs(friction.z) >= abs(m_velocity.z))
        {
            m_velocity.z = 0.0f;
        }
        else
        {
            m_velocity.z += friction.z;
        }

        // �������� ������ ������ �ӷ��� �ٽ� ���Ѵ�.
        speedXZ = GetSpeedXZ();

        float maxSpeedXZ = m_maxSpeedXZ * DT;

        // �ִ� �ӷ� ����
        if (speedXZ > maxSpeedXZ)
        {
            // �ʰ� ������ŭ �����Ѵ�.
            float ratio = maxSpeedXZ / speedXZ;

            m_velocity.x *= ratio;
            m_velocity.z *= ratio;
        }

        MovePosition();
    }

    // �̹� �����ӿ� ������ ���� �� �ʱ�ȭ
    m_force = XMFLOAT3(0.0f, 0.0f, 0.0f);
}
