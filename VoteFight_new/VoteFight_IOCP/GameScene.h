#pragma once

class CObject;

class CGameScene
{
public:
	static vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];	// 오브젝트들
	static void Load(const string& fileName);
};

