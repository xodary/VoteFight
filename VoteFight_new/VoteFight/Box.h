#pragma once
#include "Object.h"

class CBox : public CObject
{
public:
	vector<string> m_items;

	virtual void Init();
	virtual void Update();
};

class COnceItem : public CObject
{
public:
	vector<string> m_items;

	virtual void Init();
	virtual void Update();
};