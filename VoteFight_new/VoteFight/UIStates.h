#pragma once
#include "State.h"

class CMainUIState : public CState, public CSingleton<CMainUIState>
{
	friend class CSingleton<CMainUIState>;

private:
	CMainUIState();
	virtual ~CMainUIState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};

class CInventoryUIState : public CState, public CSingleton<CInventoryUIState>
{
	friend class CSingleton<CInventoryUIState>;

private:
	CInventoryUIState();
	virtual ~CInventoryUIState();

public:
	virtual void Enter(CObject* object);
	virtual void Exit(CObject* object);

	virtual void Update(CObject* object);
};