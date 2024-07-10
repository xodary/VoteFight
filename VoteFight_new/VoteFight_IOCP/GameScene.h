#pragma once

class CObject;

class CGameScene
{
public:
	static unordered_map<int, CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];	// 오브젝트들
	static float m_heights[400][400];

	static void Load(const string& fileName);
	static void LoadTerrain(const string& fileName);
	static float OnGetHeight(float fx, float fz);
};

