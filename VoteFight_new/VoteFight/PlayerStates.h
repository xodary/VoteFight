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

//=========================================================================================================================

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

//=========================================================================================================================

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

//=========================================================================================================================

class CPlayerPunchState : public CState, public CSingleton<CPlayerPunchState>
{
	friend class CSingleton;

private:
	CPlayerPunchState();
	virtual ~CPlayerPunchState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

//=========================================================================================================================

class CPlayerShootState : public CState, public CSingleton<CPlayerShootState>
{
	friend class CSingleton;

private:
	CPlayerShootState();
	virtual ~CPlayerShootState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

//=========================================================================================================================

class CPlayerDieState : public CState, public CSingleton<CPlayerDieState>
{
	friend class CSingleton;

private:
	CPlayerDieState();
	virtual ~CPlayerDieState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};
