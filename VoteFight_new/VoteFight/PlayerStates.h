#pragma once
#include "State.h"

enum class StateEnum { Idle, Walk, FocusIdle, FocusWalk };

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

class CPlayerFocusIdleState : public CState, public CSingleton<CPlayerFocusIdleState>
{
	friend class CSingleton<CPlayerFocusIdleState>;

private:
	CPlayerFocusIdleState();
	virtual ~CPlayerFocusIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CPlayerFocusWalkState : public CState, public CSingleton<CPlayerFocusWalkState>
{
	friend class CSingleton<CPlayerFocusWalkState>;

private:
	CPlayerFocusWalkState();
	virtual ~CPlayerFocusWalkState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};