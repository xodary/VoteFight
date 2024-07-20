#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"
#include "CollisionManager.h"
#include "RigidBody.h"

void CBullet::UpdatePostion()
{
    //// y�� ȸ�� ������ �������� ��ȯ
    //float rotationY = XMConvertToRadians(GetRotate().y);

    //// ȸ���� ���� ���� ���
    //XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // �⺻������ �� ������ ��Ÿ���� ����
    //XMMATRIX rotationMatrix = XMMatrixRotationY(rotationY);
    //XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    //// �̵� ���� ���
    //XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    //// ���� ��ġ ���
    //XMFLOAT3 currPosition = GetPosition();

    //// ���ο� ��ġ ���
    //XMFLOAT3 newPosition;
    //newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    //newPosition.y = currPosition.y; // y�� �̵� ����
    //newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    //// ��ġ ����
    //SetPostion(newPosition);
}

CBullet::CBullet()
{
    SetGroupType((UINT)GROUP_TYPE::BULLET);
    CreateComponent(COMPONENT_TYPE::RIGIDBODY);
}

CBullet::~CBullet()
{
    // �Ѿ� �Ҹ� �� �ʿ��� ���� �۾��� ������ �� �ֽ��ϴ�.
}


void CBullet::Init()
{
    // �Ѿ� �߻� ������ ������ �� �ֽ��ϴ�.
    m_bFired = true;
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::BULLET);
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::BULLET, GROUP_TYPE::MONSTER);
    
    XMFLOAT3 vector = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(GetRotate()));
    CRigidBody* rigidbody = reinterpret_cast<CRigidBody*>(GetComponent(COMPONENT_TYPE::RIGIDBODY));
    rigidbody->m_velocity = Vector3::ScalarProduct(vector, 40.0f);
}


void CBullet::Update()
{
    CObject::Update();
}

void CBullet::OnCollisionEnter(CObject* collidedObject)
{
    //switch (collidedObject->GetGroupType())
    //{
    //case (UINT)GROUP_TYPE::MONSTER:
    //    this->SetDeleted(true);
    //    break;
    //}

}

void CBullet::OnCollision(CObject* collidedObject)
{
}

void CBullet::OnCollisionExit(CObject* collidedObject)
{
}
