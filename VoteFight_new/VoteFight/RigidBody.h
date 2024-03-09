#pragma once
#include "Component.h"

class CRigidBody : public CComponent
{
private:
	float    m_mass;           // ����

	XMFLOAT3 m_force;		   // �� �����ӵ��� ������ ��

	XMFLOAT3 m_velocity;	   // �ӵ�
	XMFLOAT3 m_accel;		   // ���ӵ�

	float    m_maxSpeedXZ;	   // XZ�� �ִ� �ӷ�
	float    m_maxSpeedY;      // Y�� �ִ� �ӷ�

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

	void AddForce(const XMFLOAT3& force);
	void AddVelocity(const XMFLOAT3& velocity);

	virtual void Update();

private:
	void MovePosition();
};
