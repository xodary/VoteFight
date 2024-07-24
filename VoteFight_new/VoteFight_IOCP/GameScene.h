#pragma once

class CObject;

class CGameScene
{
public:

	static unordered_map<string,unordered_map<string, std::chrono::system_clock::duration>> m_animations;

	static unordered_map<int, CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];	// 오브젝트들
	static float m_heights[400][400];
	static unsigned int m_Rank[3];
	static unsigned int m_nowRank;

	static void Load(const string& fileName);
	static void LoadTerrain(const string& fileName);
	static bool can_see(XMFLOAT3 a, XMFLOAT3 b, float range);
	static float OnGetHeight(float fx, float fz);
	static void NPCInitialize();

	static void LoadSkinningAnimations();

	static bool can_see(XMFLOAT3 from, XMFLOAT3 to, int range)
	{
		if (abs(from.x - to.x) > range) return false;
		return abs(from.z - to.z) <= range;
	}

};

