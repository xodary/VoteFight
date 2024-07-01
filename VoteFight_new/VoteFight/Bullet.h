#pragma once
#include "Object.h"

class CBullet : public CObject
{
private:
    XMFLOAT3 m_direction;
    float m_fLifeTime = 1.f;
    float m_fSpeed = 3.f;
    bool m_bFired = false;

    void UpdatePostion();
public:
    CBullet();
    virtual ~CBullet();

    void Shoot();
    virtual void Update();

    virtual void OnCollisionEnter(CObject* collidedObject);
    virtual void OnCollision(CObject* collidedObject);
    virtual void OnCollisionExit(CObject* collidedObject);

};
