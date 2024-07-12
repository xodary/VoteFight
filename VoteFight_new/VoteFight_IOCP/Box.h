#pragma once
#include "Object.h"

class CBox : public CObject
{
public:
	vector<string> items;
};

class COnceItem : public CObject
{
public:
	vector<string> items;
};