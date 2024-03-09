#include "pch.h"
#include "Trigger.h"

CObject* CTrigger::m_interactionUI = nullptr;

CTrigger::CTrigger() :
    m_isInteracted(),
    m_activeFOV(360.0f),
    m_targetObjects(),
    m_event()
{
}

CTrigger::~CTrigger()
{
}

void CTrigger::SetInteractionUI(CObject* ui)
{
    m_interactionUI = ui;
}

CObject* CTrigger::GetInteractionUI()
{
    return m_interactionUI;
}

void CTrigger::SetInteracted(bool isInteracted)
{
    m_isInteracted = isInteracted;
}

bool CTrigger::IsInteracted()
{
    return m_isInteracted;
}

void CTrigger::SetActiveFOV(float FOV)
{
    m_activeFOV = FOV;
}

float CTrigger::GetActiveFOV()
{
    return m_activeFOV;
}

void CTrigger::AddTargetObject(CObject* object)
{
    if (object != nullptr)
    {
        m_targetObjects.push_back(object);
    }
}

const vector<CObject*>& CTrigger::GetTargetObjects()
{
    return m_targetObjects;
}

void CTrigger::SetEvent(Event& event)
{
    if (event != nullptr)
    {
        m_event = move(event);
    }
}

const Event& CTrigger::GetEvent()
{
    return m_event;
}

void CTrigger::Interact()
{
    if (m_event != nullptr)
    {
        m_event();
    }
}

//=========================================================================================================================

COpenDoorTrigger::COpenDoorTrigger()
{
}

COpenDoorTrigger::~COpenDoorTrigger()
{
}

void COpenDoorTrigger::OnCollisionEnter(CObject* collidedObject)
{
}

void COpenDoorTrigger::OnCollision(CObject* collidedObject)
{
}

void COpenDoorTrigger::OnCollisionExit(CObject* collidedObject)
{
}

//=========================================================================================================================

CPowerDownTrigger::CPowerDownTrigger()
{
}

CPowerDownTrigger::~CPowerDownTrigger()
{
}

void CPowerDownTrigger::OnCollisionEnter(CObject* collidedObject)
{
}

void CPowerDownTrigger::OnCollision(CObject* collidedObject)
{
}

void CPowerDownTrigger::OnCollisionExit(CObject* collidedObject)
{
}

//=========================================================================================================================

COperateSirenTrigger::COperateSirenTrigger()
{
}

COperateSirenTrigger::~COperateSirenTrigger()
{
}

void COperateSirenTrigger::OnCollisionEnter(CObject* collidedObject)
{
}

void COperateSirenTrigger::OnCollision(CObject* collidedObject)
{
}

void COperateSirenTrigger::OnCollisionExit(CObject* collidedObject)
{
}

//=========================================================================================================================

COpenGateTrigger::COpenGateTrigger()
{
}

COpenGateTrigger::~COpenGateTrigger()
{
}

void COpenGateTrigger::OnCollisionEnter(CObject* collidedObject)
{
}

void COpenGateTrigger::OnCollision(CObject* collidedObject)
{
}

void COpenGateTrigger::OnCollisionExit(CObject* collidedObject)
{
}
