#pragma once
#include "Object.h"

class CTerrainObject : public CObject
{
public:
	int m_nWidth;
	int m_nLength;

	CTerrainObject();
	~CTerrainObject();

	float heights[400][400];
	static CTerrainObject* Load(const string& fileName);

	float OnGetHeight(float x, float y);
};

