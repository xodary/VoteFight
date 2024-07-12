#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"
#include "CollisionManager.h"

void CBullet::UpdatePostion()
{
    // y축 회전 각도를 라디안으로 변환
    float rotationY = XMConvertToRadians(GetRotate().y);

    // 회전된 방향 벡터 계산
    XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // 기본적으로 앞 방향을 나타내는 벡터
    XMMATRIX rotationMatrix = XMMatrixRotationY(rotationY);
    XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    // 이동 벡터 계산
    XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    // 현재 위치 얻기
    XMFLOAT3 currPosition = GetPostion();

    // 새로운 위치 계산
    XMFLOAT3 newPosition;
    newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    newPosition.y = currPosition.y; // y축 이동 없음
    newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    // 위치 설정
    SetPostion(newPosition);
}

CBullet::CBullet()
{
    SetGroupType((UINT)GROUP_TYPE::BULLET);
}

CBullet::~CBullet()
{
    // 총알 소멸 시 필요한 정리 작업을 수행할 수 있습니다.
}


void CBullet::Shoot()
{
    // 총알 발사 동작을 정의할 수 있습니다.
    m_bFired = true;
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::BULLET);
    CCollisionManager::GetInstance()->SetCollisionGroup(GROUP_TYPE::BULLET, GROUP_TYPE::MONSTER);
}


void CBullet::Update()
{
    CObject::Update();
    // 타이머 매니저에서 델타 타임을 가져와서 m_fLifeTime을 줄입니다.
    float deltaTime = CTimeManager::GetInstance()->GetDeltaTime();

    if (m_fLifeTime > 0.0f && m_bFired)
    {
        cout << "업데이트" << endl;
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
