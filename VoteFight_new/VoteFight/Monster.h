#pragma once
#include "Character.h"

class CMonster : public CCharacter
{
private:
    string	 m_spineName;
    float	 m_spineAngle;
    float    m_turnAngle;

    bool	 m_isAiming;
    CObject* AimObejct;
    float m_fSpeed = 0.01f;

public:

    bool m_dead;

    CMonster();
    virtual ~CMonster();

    string GetSpineName();
    float GetSpineAngle();
    void SetSpineAngle(float angle);

    virtual void Init();
    virtual void Update();
    virtual void PlayerDiscovery(CObject* player);
    virtual void OnCollisionEnter(CObject* collidedObject);
    virtual void OnCollisionExit(CObject* collidedObject);
};
