#pragma once
#include "Character.h"

class CPlayer : public CCharacter
{
private:
	bool	 m_isAiming;
	int	     m_bulletCount;
	string	 m_spineName;
	float	 m_spineAngle;
	float    m_turnAngle;

public:
	CPlayer();
	virtual ~CPlayer();

	void SetTurnAngle(float look);
	float GetTurnAngle();
	string GetSpineName();
	float GetSpineAngle();
	void SetSpineAngle(float angle);
	void SetAiming(bool isAiming);
	bool IsAiming();

	bool HasBullet();

	virtual void Init();

	virtual void SwapWeapon(WEAPON_TYPE weaponType);

	void Punch();
	void Shoot();

	virtual void Update();

	//void ManagePistol(bool HasPistol);
	//bool HasPistol() const;
	//bool IsEquippedPistol() const;

	//void ManageKey(bool HasKey);
	//bool HasKey() const;

	//bool SwapWeapon(WEAPON_TYPE WeaponType);

	//void Rotate(float Pitch, float Yaw, float Roll, float ElapsedTime, float NearestHitDistance);

	//void IsCollidedByEventTrigger(const XMFLOAT3& NewPosition);
};
