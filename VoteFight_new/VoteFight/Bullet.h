#pragma once
#include "Object.h"

class CBullet : public CObject
{
public:
    CBullet();
    ~CBullet();

    // ������ ���� �ʿ��� �߰� ����� ������ �� �ֽ��ϴ�.
    void Shoot();
    void Update() override; // �������̵��Ͽ� �Ѿ��� �������� ������ �� �ֽ��ϴ�.
};
