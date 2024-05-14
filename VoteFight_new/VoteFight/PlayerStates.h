#pragma once
#include "State.h"

class CPlayerIdleState : public CState, public CSingleton<CPlayerIdleState>
{
	friend class CSingleton<CPlayerIdleState>;

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
	friend class CSingleton<CPlayerWalkState>;

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
	friend class CSingleton<CPlayerRunState>;

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
	friend class CSingleton<CPlayerLeftTurn>;
	float restAngle;
	float lookAngle;
	float speed;

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
	friend class CSingleton<CPlayerRightTurn>;
	float restAngle;
	float lookAngle;
	float speed;

private:
	CPlayerRightTurn();
	virtual ~CPlayerRightTurn();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};