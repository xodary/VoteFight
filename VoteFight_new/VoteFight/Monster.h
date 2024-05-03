#pragma once
#include "Character.h"

class CMonster : public CCharacter
{
private:
    string	 m_spineName;
    float	 m_spineAngle;
    float    m_turnAngle;

public:
    CMonster();
    virtual ~CMonster();

    string GetSpineName();
    float GetSpineAngle();
    void SetSpineAngle(float angle);

    virtual void Init();
    virtual void Update();
};
