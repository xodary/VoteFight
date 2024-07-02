#pragma once
#include "object.h"

class CWeapon : public CObject
{
public:
	int m_nDamage;

	CWeapon();
	~CWeapon();

	virtual void Attack(CObject* object) { cout << "attacked" << endl; }
};

class CGun : public CWeapon
{
public:
	int m_nBullet;

	CGun();
	~CGun();

	virtual void Attack(CObject* object);
};

class CSword : public CWeapon
{
public:
	CSword();
	~CSword();

	virtual void Attack(CObject* object);
};