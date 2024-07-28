#pragma once
#include "Object.h"

class CItem : public CObject
{
public:
	string	m_ItemName;
	int		m_Capacity;

	CItem();
};

