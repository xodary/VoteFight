#include "pch.h"
#include "TerrainObject.h"
#include "AssetManager.h"

CTerrainObject::CTerrainObject()
{
	m_nWidth = m_nLength = 400;
}

CTerrainObject::~CTerrainObject()
{
}

CTerrainObject* CTerrainObject::Load(const string& fileName)
{
	CTerrainObject* terrain = new CTerrainObject();
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Terrain\\" + fileName + ".bin";
	ifstream in(filePath, ios::binary);
	int num;
	in.read(reinterpret_cast<char*>(&num), sizeof(int));

	for (int i = 0; i < num; ++i)
	{
		int x = 0;
		int z = 0;
		in.read(reinterpret_cast<char*>(&x), sizeof(int));
		in.read(reinterpret_cast<char*>(&z), sizeof(int));
		float height = 0;
		in.read(reinterpret_cast<char*>(&height), sizeof(float));

		if (0 <= x && x < 400 && 0 <= z && z < 400)
			terrain->heights[x][z] = height;
	}

	for (int i = 0; i < 400; ++i)
	{
		for (int j = 0; j < 400; ++j)
		{
			if (terrain->heights[i][j] < EPSILON) {
				if(i-1 >= 0) terrain->heights[i][j] = terrain->heights[i-1][j];
				else terrain->heights[i][j] = terrain->heights[i][j-1];
			}
		}
	}

	return terrain;
}

float CTerrainObject::OnGetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = heights[x][z];
	float fBottomRight = heights[x + 1][z];
	float fTopLeft = heights[x][z + 1];
	float fTopRight = heights[x + 1][z + 1];

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	printf("%lf, %lf, %lf\n", fx, fz, fHeight);
	return(fHeight);
}