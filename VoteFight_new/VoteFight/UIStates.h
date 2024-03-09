#pragma once
#include "State.h"

class CMissionUIShowState : public CState, public CSingleton<CMissionUIShowState>
{
	friend class CSingleton;

private:
	CMissionUIShowState();
	virtual ~CMissionUIShowState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

//=========================================================================================================================

class CMissionUIHideState : public CState, public CSingleton<CMissionUIHideState>
{
	friend class CSingleton;

private:
	CMissionUIHideState();
	virtual ~CMissionUIHideState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

//=========================================================================================================================

class CKeyUIActivateState : public CState, public CSingleton<CKeyUIActivateState>
{
	friend class CSingleton;

private:
	CKeyUIActivateState();
	virtual ~CKeyUIActivateState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

//=========================================================================================================================

class CHitUIFadeState : public CState, public CSingleton<CHitUIFadeState>
{
	friend class CSingleton;

private:
	CHitUIFadeState();
	virtual ~CHitUIFadeState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};