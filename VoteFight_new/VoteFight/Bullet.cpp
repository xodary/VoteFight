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
    // 총알 생성 시 필요한 초기화 작업을 수행할 수 있습니다.
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
    // 총알 소멸 시 필요한 정리 작업을 수행할 수 있습니다.
}

void CBullet::Shoot()
{
    // 총알 발사 동작을 정의할 수 있습니다.
}


void CBullet::Update()
{
    // 타이머 매니저에서 델타 타임을 가져와서 m_fLifeTime을 줄입니다.
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

    // 총알의 위치를 업데이트합니다.
    UpdatePostion();
}