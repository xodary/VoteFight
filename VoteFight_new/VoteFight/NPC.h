#pragma once
#include "Character.h"

class CNPC : public CCharacter
{
private:
	string	 m_spineName;
	float	 m_spineAngle;
	float    m_turnAngle;

public:
	CNPC();
	virtual ~CNPC();
	
	string GetSpineName();
	float GetSpineAngle();
	void SetSpineAngle(float angle);

	virtual void Init();
	virtual void Update();
	virtual void OnCollisionEnter(CObject* collidedObject);
};
