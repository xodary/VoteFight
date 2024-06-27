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

    // ������ ���� �ʿ��� �߰� ����� ������ �� �ֽ��ϴ�.
    void Shoot();
    virtual void Update();
    void SetDirection(const XMFLOAT3& direction) ; // �������̵��Ͽ� �Ѿ��� �������� ������ �� �ֽ��ϴ�.
};
