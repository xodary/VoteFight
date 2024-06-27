#pragma once
#include "Object.h"

class CBullet : public CObject
{
private:
    XMFLOAT3 m_direction;
    float m_fLifeTime = 1.f;
    float m_fSpeed = 3.f;

    void UpdatePostion();
public:
    CBullet();
    virtual ~CBullet();

    // 다음과 같이 필요한 추가 기능을 정의할 수 있습니다.
    void Shoot();
    virtual void Update();
    void SetDirection(const XMFLOAT3& direction) ; // 오버라이딩하여 총알의 움직임을 정의할 수 있습니다.
};
