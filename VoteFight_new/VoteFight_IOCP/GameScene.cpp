#include "pch.h"
#include "GameScene.h"
#include "Object.h"

float CGameScene::m_heights[400][400];

float CGameScene::OnGetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= 400) || (fz >= 400)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = m_heights[x][z];
	float fBottomRight = m_heights[x + 1][z];
	float fTopLeft = m_heights[x][z + 1];
	float fTopRight = m_heights[x + 1][z + 1];

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

void CGameScene::Load(const string& fileName)
{
	char assetPath[255] = {};
	GetCurrentDirectoryA(255, assetPath);
	for (int i = static_cast<int>(strlen(assetPath) - 1); i >= 0; --i)
	{
		if (assetPath[i] == '\\')
		{
			assetPath[i] = '\0';

			break;
		}
	}
	strcat_s(assetPath, 255, "\\Release\\Asset\\");
	string m_assetPath = assetPath;

	string filePath = m_assetPath + "Scene\\" + fileName;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << filePath << " : Can't Find tihs File." << std::endl;
		return;
	}

	string str, modelFileName;
	GROUP_TYPE groupType = {};

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<GroupType>")
		{
			in.read(reinterpret_cast<char*>(&groupType), sizeof(int));
		}
		else if (str == "<FileName>")
		{
			File::ReadStringFromFile(in, modelFileName);
		}
		else if (str == "<Instance>")
		{
			int instanceCount = 0;

			in.read(reinterpret_cast<char*>(&instanceCount), sizeof(int));

			// <IsActive>
			File::ReadStringFromFile(in, str);

			vector<int> isActives(instanceCount);

			in.read(reinterpret_cast<char*>(isActives.data()), instanceCount * sizeof(int));

			// <Transforms>
			File::ReadStringFromFile(in, str);

			// localPosition, localRotation, localScale
			vector<XMFLOAT3> transforms(3 * instanceCount);

			in.read(reinterpret_cast<char*>(transforms.data()), 3 * instanceCount * sizeof(XMFLOAT3));

			switch (groupType)
			{
			case GROUP_TYPE::ITEM:
			case GROUP_TYPE::NPC:
			case GROUP_TYPE::MONSTER:
				cout << modelFileName << " is Loading..." << endl;
				for (int i = 0; i < instanceCount; ++i)
				{
					CObject* object = new CObject();
					object->m_grouptype = (int)groupType;
					object->m_modelname = modelFileName;
					object->m_Pos = transforms[3 * i];
					object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
					object->m_Rota = transforms[3 * i + 1];
					object->m_id = m_objects[static_cast<int>(groupType)].size();

					m_objects[static_cast<int>(groupType)][object->m_id] = object;
				}
				break;
			}
		}
		else if (str == "</Scene>")
		{
			cout << endl;
			break;
		}
	}
}

void CGameScene::LoadTerrain(const string& fileName)
{
	char assetPath[255] = {};
	GetCurrentDirectoryA(255, assetPath);
	for (int i = static_cast<int>(strlen(assetPath) - 1); i >= 0; --i)
	{
		if (assetPath[i] == '\\')
		{
			assetPath[i] = '\0';

			break;
		}
	}
	strcat_s(assetPath, 255, "\\Release\\Asset\\");
	string m_assetPath = assetPath;

	string filePath = m_assetPath + "Terrain\\" + fileName;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << filePath << " : Can't Find tihs File." << std::endl;
		return;
	}

	for (int i = 0; i < 400; ++i)
	{
		for (int j = 0; j < 400; ++j)
		{
			float height = 0;
			in.read(reinterpret_cast<char*>(&height), sizeof(float));
			m_heights[i][j] = height;
		}
	}

}