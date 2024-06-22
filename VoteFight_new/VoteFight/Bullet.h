#pragma once
#include "Object.h"

class CBullet : public CObject
{
private:
    XMFLOAT3 m_direction;
    float m_fLifeTime;
    float m_fSpeed = 0.01f;
    float m_Hp;

    void UpdatePostion();
public:
    CBullet();
    CBullet(XMFLOAT3 postion, XMFLOAT3 direction);
    virtual ~CBullet();

    // ������ ���� �ʿ��� �߰� ����� ������ �� �ֽ��ϴ�.
    void Shoot();
    void Update() override; // �������̵��Ͽ� �Ѿ��� �������� ������ �� �ֽ��ϴ�.
};
