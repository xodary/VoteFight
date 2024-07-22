#pragma once
#include "Object.h"

class CStateMachine;
class RemoteClient;

class CMonster : public CObject
{
public:
	int					m_meet;
	CStateMachine*		m_stateMachine;
	RemoteClient*		m_target;

	CMonster();
	~CMonster();
};

