#pragma once
#include "Object.h"

class CMonster : public CObject
{
public:
	int		m_Health;
	bool	m_dead;

	CMonster();
	~CMonster();
};

