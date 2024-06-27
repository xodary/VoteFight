#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"

void CBullet::UpdatePostion()
{
    // ȸ���� ���� ���� ���
    XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // �⺻������ �� ������ ��Ÿ���� ����
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&GetRotate()));
    XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    // �̵� ���� ���
    XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    // ���� ��ġ ���
    XMFLOAT3 currPosition = GetPostion();

    // ���ο� ��ġ ���
    XMFLOAT3 newPosition;
    newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    newPosition.y = currPosition.y + XMVectorGetY(moveVector);
    newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    // ��ġ ����
    SetPostion(newPosition);
}

CBullet::CBullet()
{
}

CBullet::~CBullet()
{
    // �Ѿ� �Ҹ� �� �ʿ��� ���� �۾��� ������ �� �ֽ��ϴ�.
}


void CBullet::Shoot()
{
    // �Ѿ� �߻� ������ ������ �� �ֽ��ϴ�.
}


void CBullet::Update()
{
    CObject::Update();
    cout << "�߻���..." << endl;
    // Ÿ�̸� �Ŵ������� ��Ÿ Ÿ���� �����ͼ� m_fLifeTime�� ���Դϴ�.
    float deltaTime = CTimeManager::GetInstance()->GetDeltaTime();
    UpdatePostion();

    if (m_fLifeTime <= 0.0f)
    {
        this->SetDeleted(true);
    }
    else
    {
            m_fLifeTime -= deltaTime;
    }
}

void CBullet::SetDirection(const XMFLOAT3& direction)
{
    m_direction = direction;
}
