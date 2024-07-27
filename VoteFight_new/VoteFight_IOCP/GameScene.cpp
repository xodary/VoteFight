#include "pch.h"
#include "GameScene.h"
#include "NPC.h"
#include "Object.h"
#include "Box.h"

float CGameScene::m_heights[400][400];

bool CGameScene::can_see(XMFLOAT3 a, XMFLOAT3 b, float range)
{
	if (abs(a.x - b.x) > range) return false;
	return abs(a.z - b.z) <= range;
}

float CGameScene::OnGetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= 400) || (fz >= 400)) return -6.f;

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
			case GROUP_TYPE::STRUCTURE:
			case GROUP_TYPE::NPC:
			case GROUP_TYPE::MONSTER:
			case GROUP_TYPE::BOX:
			case GROUP_TYPE::ONCE_ITEM:
				cout << modelFileName << " is Loading..." << endl;
				for (int i = 0; i < instanceCount; ++i)
				{
					CObject* object = CObject::Load(modelFileName);

					object->m_grouptype = (int)groupType;
					object->m_modelname = modelFileName;
					object->m_Pos = transforms[3 * i];
					object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
					object->m_Rota = transforms[3 * i + 1];
					object->m_Angle = object->m_Rota.y;
					object->m_Sca = transforms[3 * i + 2];
					object->m_id = m_objects[static_cast<int>(groupType)].size();

					if (object->m_collider) {
						XMFLOAT4X4 matrix = Matrix4x4::Identity();
						matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Scale(object->m_Sca));
						matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Rotation(object->m_Rota));
						matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object->m_Pos));
						object->m_origin.Transform(object->m_boundingBox, XMLoadFloat4x4(&matrix));
					}

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

void CGameScene::NPCInitialize()
{
	cout << "NPCInitialize()" << endl;

	string needs_ex[] = {"fish_meet", "wood", "flower", "trash", "carrot", "icecream", "potato" };
	string output_ex[] = { "election_ticket", "drug", "bullets" };
	for (auto& object : m_objects[(int)GROUP_TYPE::NPC])
	{
		CNPC* npc = reinterpret_cast<CNPC*>(object.second);
		
		int n = rand() % 3 + 1;
		for (int i = 0; i < n; ++i)
		{
			npc->m_needs.push_back(needs_ex[rand() % size(needs_ex)]);
		}

		int o = rand() % 2 + 1;
		for (int i = 0; i < o; ++i)
		{
			npc->m_outputs.push_back(output_ex[rand() % size(output_ex)]);
		}
	}

	for (auto& object : m_objects[(int)GROUP_TYPE::ONCE_ITEM])
	{
		COnceItem* onceitem = reinterpret_cast<COnceItem*>(object.second);
		onceitem->items.push_back("flower");
	}

	for (auto& object : m_objects[(int)GROUP_TYPE::BOX])
	{
		CBox* box = reinterpret_cast<CBox*>(object.second);
		int o = rand() % 3 + 1;
		for (int i = 0; i < o; ++i)
		{
			box->items.push_back(needs_ex[rand() % size(needs_ex)]);
		}
	}
}

void CGameScene::LoadSkinningAnimations()
{
	string models[] { "FishMon","Solider_Homer","Homer_Link","Marge_Police","hugo_idle","Sonic","Mario" };
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

	for (auto modelname : models)
	{
		string filePath = m_assetPath + "Animation\\" + modelname + "_Animation.bin";
		ifstream in(filePath, ios::binary);
		string str;

		while (true)
		{
			File::ReadStringFromFile(in, str);

			if (str == "<Animations>")
			{
				int animationCount = 0;
				in.read(reinterpret_cast<char*>(&animationCount), sizeof(int));
			}
			else if (str == "<Animation>")
			{
				string str;
				string name;
				int skinnedMeshCount = 0;

				while (true)
				{
					File::ReadStringFromFile(in, str);

					if (str == "<Name>")
					{
						File::ReadStringFromFile(in, name);
					}
					else if (str == "<FrameRate>")
					{
						int a;
						in.read(reinterpret_cast<char*>(&a), sizeof(int));
					}
					else if (str == "<FrameCount>")
					{
						int a;
						in.read(reinterpret_cast<char*>(&a), sizeof(int));
					}
					else if (str == "<Duration>")
					{
						float time;
						in.read(reinterpret_cast<char*>(&time), sizeof(float));
						std::chrono::duration<float> duration_in_seconds(time);
						m_animations[modelname][name] = chrono::duration_cast<chrono::system_clock::duration>(duration_in_seconds);
					}
					else if (str == "<SkinnedMeshes>")
					{
						in.read(reinterpret_cast<char*>(&skinnedMeshCount), sizeof(int));
					}
					else if (str == "<ElapsedTime>")
					{
						float elapsedTime = 0.0f;

						in.read(reinterpret_cast<char*>(&elapsedTime), sizeof(float));

						for (int i = 0; i < skinnedMeshCount; ++i)
						{
							// <BoneTransformMatrix>
							File::ReadStringFromFile(in, str);

							int boneCount = 0;

							in.read(reinterpret_cast<char*>(&boneCount), sizeof(int));

							for (int j = 0; j < boneCount; ++j)
							{
								// localPosition, localRotation, localScale
								XMFLOAT3 transform[3] = {};

								in.read(reinterpret_cast<char*>(&transform[0]), 3 * sizeof(XMFLOAT3));
							}
						}
					}
					else if (str == "</Animation>")
					{
						break;
					}
				}
			}
			else if (str == "</Animations>")
			{
				break;
			}
		}
	}
}
