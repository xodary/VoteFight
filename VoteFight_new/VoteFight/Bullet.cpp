#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"
#include "CollisionManager.h"

void CBullet::UpdatePostion()
{
    // y�� ȸ�� ������ �������� ��ȯ
    float rotationY = XMConvertToRadians(GetRotate().y);

    // ȸ���� ���� ���� ���
    XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // �⺻������ �� ������ ��Ÿ���� ����
    XMMATRIX rotationMatrix = XMMatrixRotationY(rotationY);
    XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    // �̵� ���� ���
    XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    // ���� ��ġ ���
    XMFLOAT3 currPosition = GetPostion();

    // ���ο� ��ġ ���
    XMFLOAT3 newPosition;
    newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    newPosition.y = currPosition.y; // y�� �̵� ����
    newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    // ��ġ ����
    SetPostion(newPosition);
}

CBullet::CBullet()
{
    SetGroupType((UINT)GROUP_TYPE::BULLET);
}

CBullet::~CBullet()
{
    // �Ѿ� �Ҹ� �� �ʿ��� ���� �۾��� ������ �� �ֽ��ϴ�.
}


void CBullet::Shoot()
{
    // �Ѿ� �߻� ������ ������ �� �ֽ��ϴ�.
    m_bFired = true;
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::BULLET);
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::BULLET, GROUP_TYPE::MONSTER);
}


void CBullet::Update()
{
    CObject::Update();
    // Ÿ�̸� �Ŵ������� ��Ÿ Ÿ���� �����ͼ� m_fLifeTime�� ���Դϴ�.
    float deltaTime = CTimeManager::GetInstance()->GetDeltaTime();

    if (m_fLifeTime > 0.0f && m_bFired)
    {
        cout << "������Ʈ" << endl;
        UpdatePostion();
        m_fLifeTime -= deltaTime;
    }
    else
    {
        this->SetDeleted(true);
    }
}

void CBullet::OnCollisionEnter(CObject* collidedObject)
{
    switch (collidedObject->GetGroupType())
    {
    case (UINT)GROUP_TYPE::MONSTER:
        this->SetDeleted(true);
        break;
    }

}

void CBullet::OnCollision(CObject* collidedObject)
{
}

void CBullet::OnCollisionExit(CObject* collidedObject)
{
}
