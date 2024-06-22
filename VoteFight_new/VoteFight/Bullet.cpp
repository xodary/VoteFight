#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"

void CBullet::UpdatePostion()
{
    XMFLOAT3 currPostion = GetPostion();
    XMFLOAT3 addPostion = m_direction;
    addPostion.x *= m_fSpeed;
    addPostion.y *= m_fSpeed;
    addPostion.z *= m_fSpeed;
    currPostion.x += addPostion.x;
    currPostion.y += addPostion.y;
    currPostion.z += addPostion.z;

    SetPostion(currPostion);
}

CBullet::CBullet()
{
    // �Ѿ� ���� �� �ʿ��� �ʱ�ȭ �۾��� ������ �� �ֽ��ϴ�.
}

CBullet::CBullet(XMFLOAT3 postion, XMFLOAT3 direction)
{
    SetMesh();
    SetPostion(postion);
    SetRotate(direction);
    m_direction = direction;
    m_fLifeTime = 5.0f;
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
    // Ÿ�̸� �Ŵ������� ��Ÿ Ÿ���� �����ͼ� m_fLifeTime�� ���Դϴ�.
    float deltaTime = CTimeManager::GetInstance()->GetDeltaTime();

    if (m_fLifeTime <= 0.0f)
    {
        this->SetDeleted(true);
    }
    else
    {
        UpdatePostion();
        m_fLifeTime -= deltaTime;
    }

    // �Ѿ��� ��ġ�� ������Ʈ�մϴ�.
    UpdatePostion();
}