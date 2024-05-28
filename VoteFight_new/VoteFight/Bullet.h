#pragma once
#include "Object.h"

class CBullet : public CObject
{
public:
    CBullet();
    ~CBullet();

    // 다음과 같이 필요한 추가 기능을 정의할 수 있습니다.
    void Shoot();
    void Update() override; // 오버라이딩하여 총알의 움직임을 정의할 수 있습니다.
};
