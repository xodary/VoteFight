#pragma once
#include "Object.h"

class CNPC : public CObject
{
public:
	vector<string> m_needs;
	vector<string> m_outputs;
};

