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
	float	 m_clickAngle;

public:
	
	CPlayer();
	virtual ~CPlayer();

	void SetClickAngle(float clickAngle);
	float GetClickAngle();
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

	BYTE MoveDirection = 0x0000000;
	bool b_front = false;
	bool b_right = false;
	bool b_back = false;
	bool b_left = false;
	//void ManagePistol(bool HasPistol);
	//bool HasPistol() const;
	//bool IsEquippedPistol() const;

	//void ManageKey(bool HasKey);
	//bool HasKey() const;

	//bool SwapWeapon(WEAPON_TYPE WeaponType);

	//void Rotate(float Pitch, float Yaw, float Roll, float ElapsedTime, float NearestHitDistance);

	//void IsCollidedByEventTrigger(const XMFLOAT3& NewPosition);
};
