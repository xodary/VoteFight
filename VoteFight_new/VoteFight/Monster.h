#pragma once
#include "Character.h"

class CMonster : public CCharacter
{
public:
    bool	 m_isAiming;
    CObject* AimObejct;

    float goal_rota;

    bool m_dead;

    CMonster();
    virtual ~CMonster();

    virtual void Init();
    virtual void Update();
};
