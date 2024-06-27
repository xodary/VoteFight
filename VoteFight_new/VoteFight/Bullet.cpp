#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"

void CBullet::UpdatePostion()
{
    // 회전된 방향 벡터 계산
    XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // 기본적으로 앞 방향을 나타내는 벡터
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&GetRotate()));
    XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    // 이동 벡터 계산
    XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    // 현재 위치 얻기
    XMFLOAT3 currPosition = GetPostion();

    // 새로운 위치 계산
    XMFLOAT3 newPosition;
    newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    newPosition.y = currPosition.y + XMVectorGetY(moveVector);
    newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    // 위치 설정
    SetPostion(newPosition);
}

CBullet::CBullet()
{
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
    CObject::Update();
    cout << "발사중..." << endl;
    // 타이머 매니저에서 델타 타임을 가져와서 m_fLifeTime을 줄입니다.
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
