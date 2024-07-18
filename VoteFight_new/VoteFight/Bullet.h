#pragma once
#include "Object.h"

class CBullet : public CObject
{
private:
    XMFLOAT3 m_direction;
    float m_fLifeTime = 3.f;
    float m_fSpeed = 1.0f;
    bool m_bFired = false;

    void UpdatePostion();
public:
    CBullet();
    virtual ~CBullet();

    virtual void Init();
    virtual void Update();

    virtual void OnCollisionEnter(CObject* collidedObject);
    virtual void OnCollision(CObject* collidedObject);
    virtual void OnCollisionExit(CObject* collidedObject);

};
