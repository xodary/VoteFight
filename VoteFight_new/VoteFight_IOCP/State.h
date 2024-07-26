#pragma once

class CObject;

class CState
{
protected:
	CState();
	virtual ~CState();

public:
	virtual void Enter(CObject* object) = 0;
	virtual void Exit(CObject* object) = 0;

	virtual void Update(CObject* object) = 0;
};

class CMonsterIdleState : public CState, public CSingleton<CMonsterIdleState>
{
	friend class CSingleton<CMonsterIdleState>;

private:
	CMonsterIdleState();
	virtual ~CMonsterIdleState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CMonsterWalkState : public CState, public CSingleton<CMonsterWalkState>
{
	friend class CSingleton<CMonsterWalkState>;

private:
	CMonsterWalkState();
	virtual ~CMonsterWalkState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CMonsterChaseState : public CState, public CSingleton<CMonsterChaseState>
{
	friend class CSingleton<CMonsterChaseState>;

private:
	CMonsterChaseState();
	virtual ~CMonsterChaseState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CMonsterAttackState : public CState, public CSingleton<CMonsterAttackState>
{
	friend class CSingleton<CMonsterAttackState>;

private:
	CMonsterAttackState();
	virtual ~CMonsterAttackState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CMonsterAttackedState : public CState, public CSingleton<CMonsterAttackedState>
{
	friend class CSingleton<CMonsterAttackedState>;

private:
	CMonsterAttackedState();
	virtual ~CMonsterAttackedState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

