#pragma once

class CObject;

class CGameScene
{
public:
	static vector<CObject*> m_objects[static_cast<int>(GROUP_TYPE::COUNT)];	// ������Ʈ��
	static void Load(const string& fileName);
};

