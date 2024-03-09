#pragma once
#include "Object.h"

using Event = function<void()>;

class CTrigger : public CObject
{
protected:
	static CObject*  m_interactionUI;

	bool			 m_isInteracted;
	float			 m_activeFOV;

	vector<CObject*> m_targetObjects;

	Event			 m_event;

public:
	CTrigger();
	virtual ~CTrigger();

	static void SetInteractionUI(CObject* ui);
	static CObject* GetInteractionUI();

	void SetInteracted(bool isInteracted);
	bool IsInteracted();

	void SetActiveFOV(float FOV);
	float GetActiveFOV();

	void AddTargetObject(CObject* object);
	const vector<CObject*>& GetTargetObjects();

	void SetEvent(Event& event);
	const Event& GetEvent();

	virtual void Interact();
};

//=========================================================================================================================

class COpenDoorTrigger : public CTrigger
{
public:
	COpenDoorTrigger();
	virtual ~COpenDoorTrigger();

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);
};

//=========================================================================================================================

class CPowerDownTrigger : public CTrigger
{
public:
	CPowerDownTrigger();
	virtual ~CPowerDownTrigger();

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);
};

//=========================================================================================================================

class COperateSirenTrigger : public CTrigger
{
public:
	COperateSirenTrigger();
	virtual ~COperateSirenTrigger();

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);
};

//=========================================================================================================================

class COpenGateTrigger : public CTrigger
{
public:
	COpenGateTrigger();
	virtual ~COpenGateTrigger();

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);
};
