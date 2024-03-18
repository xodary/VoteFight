#pragma once
#include "State.h"

class CPlayerIdleState : public CState, public CSingleton<CPlayerIdleState>
{
	friend class CSingleton;

private:
	CPlayerIdleState();
	virtual ~CPlayerIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CPlayerWalkState : public CState, public CSingleton<CPlayerWalkState>
{
	friend class CSingleton;

private:
	CPlayerWalkState();
	virtual ~CPlayerWalkState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CPlayerRunState : public CState, public CSingleton<CPlayerRunState>
{
	friend class CSingleton;

private:
	CPlayerRunState();
	virtual ~CPlayerRunState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CPlayerLeftTurn : public CState, public CSingleton<CPlayerLeftTurn>
{
	friend class CSingleton;
	float restAngle;
	float lookAngle;

private:
	CPlayerLeftTurn();
	virtual ~CPlayerLeftTurn();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CPlayerRightTurn : public CState, public CSingleton<CPlayerRightTurn>
{
	friend class CSingleton;
	float restAngle;
	float lookAngle;

private:
	CPlayerRightTurn();
	virtual ~CPlayerRightTurn();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};