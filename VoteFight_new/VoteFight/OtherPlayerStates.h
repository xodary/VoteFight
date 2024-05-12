#pragma once
#include "State.h"

class COtherPlayerIdleState : public CState, public CSingleton<COtherPlayerIdleState>
{
	friend class CSingleton<COtherPlayerIdleState>;

private:
	COtherPlayerIdleState();
	virtual ~COtherPlayerIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class COtherPlayerWalkState : public CState, public CSingleton<COtherPlayerWalkState>
{
	friend class CSingleton<COtherPlayerWalkState>;

private:
	COtherPlayerWalkState();
	virtual ~COtherPlayerWalkState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class COhterPlayerRunState : public CState, public CSingleton<COhterPlayerRunState>
{
	friend class CSingleton<COhterPlayerRunState>;

private:
	COhterPlayerRunState();
	virtual ~COhterPlayerRunState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class COtherPlayerLeftTurn : public CState, public CSingleton<COtherPlayerLeftTurn>
{
	friend class CSingleton<COtherPlayerLeftTurn>;
	float restAngle;
	float lookAngle;

private:
	COtherPlayerLeftTurn();
	virtual ~COtherPlayerLeftTurn();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class COtherPlayerRightTurn : public CState, public CSingleton<COtherPlayerRightTurn>
{
	friend class CSingleton<COtherPlayerRightTurn>;
	float restAngle;
	float lookAngle;
private:
	COtherPlayerRightTurn();
	virtual ~COtherPlayerRightTurn();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};