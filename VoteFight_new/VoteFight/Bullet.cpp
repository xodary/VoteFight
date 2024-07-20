#include "pch.h"
#include "Bullet.h"
#include "TimeManager.h"
#include "CollisionManager.h"
#include "RigidBody.h"

void CBullet::UpdatePostion()
{
    //// y축 회전 각도를 라디안으로 변환
    //float rotationY = XMConvertToRadians(GetRotate().y);

    //// 회전된 방향 벡터 계산
    //XMVECTOR forwardDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // 기본적으로 앞 방향을 나타내는 벡터
    //XMMATRIX rotationMatrix = XMMatrixRotationY(rotationY);
    //XMVECTOR rotatedForward = XMVector3TransformCoord(forwardDirection, rotationMatrix);

    //// 이동 벡터 계산
    //XMVECTOR moveVector = rotatedForward * m_fSpeed * CTimeManager::GetInstance()->GetDeltaTime();

    //// 현재 위치 얻기
    //XMFLOAT3 currPosition = GetPosition();

    //// 새로운 위치 계산
    //XMFLOAT3 newPosition;
    //newPosition.x = currPosition.x + XMVectorGetX(moveVector);
    //newPosition.y = currPosition.y; // y축 이동 없음
    //newPosition.z = currPosition.z + XMVectorGetZ(moveVector);

    //// 위치 설정
    //SetPostion(newPosition);
}

CBullet::CBullet()
{
    SetGroupType((UINT)GROUP_TYPE::BULLET);
    CreateComponent(COMPONENT_TYPE::RIGIDBODY);
}

CBullet::~CBullet()
{
    // 총알 소멸 시 필요한 정리 작업을 수행할 수 있습니다.
}


void CBullet::Init()
{
    // 총알 발사 동작을 정의할 수 있습니다.
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
