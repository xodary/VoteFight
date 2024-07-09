#pragma once
#include "Component.h"

class CRigidBody : public CComponent
{
public:
	float    m_mass;           // 질량

	XMFLOAT3 m_force;		   // 한 프레임동안 누적된 힘

	XMFLOAT3 m_velocity;	   // 속도
	XMFLOAT3 m_accel;		   // 가속도

	float    m_maxSpeedXZ;	   // XZ축 최대 속력
	float    m_maxSpeedY;      // Y축 최대 속력

	bool isMovable = true;
public:
	CRigidBody();
	virtual ~CRigidBody();

	void SetMass(float mass);
	float GetMass();

	const XMFLOAT3& GetVelocity();

	void SetMaxSpeedXZ(float maxSpeedXZ);
	float GetMaxSpeedXZ();

	void SetMaxSpeedY(float maxSpeedY);
	float GetMaxSpeedY();

	float GetSpeedXZ();
	float GetSpeedY();

	void AddForce(const XMFLOAT3& force, bool isMovable);
	void AddVelocity(const XMFLOAT3& velocity);

	XMFLOAT3 GetAfterMovePosition();
	void ReturnPrevLocation(const XMFLOAT3& velocity);
	virtual void Update();

private:
	void MovePosition();
};
