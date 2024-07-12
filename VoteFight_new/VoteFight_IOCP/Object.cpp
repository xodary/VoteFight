#include "pch.h"
#include "Object.h"
#include "CPlayer.h"
#include "NPC.h"
#include "Box.h"


CObject* CObject::Load(const string& fileName)
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

	string filePath = m_assetPath + "Model\\" + fileName + ".bin";
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cerr << filePath << " : Can't Find tihs File." << std::endl;
	}

	CObject* object = {};
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Frames>")
		{
			object = CObject::LoadFrame(in);
		}
		else if (str == "</Frames>")
		{
			break;
		}
	}

	File::ReadStringFromFile(in, str);

	//if (str == "<Animator>")
	//{
	//	CAnimator* animator = static_cast<CAnimator*>(object->CreateComponent(COMPONENT_TYPE::ANIMATOR));

	//	animator->Load(in);
	//}

	return object;
}

CObject* CObject::LoadFrame(ifstream& in)
{
	CObject* object = nullptr;
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<ClassType>")
		{
			int classType = 0;

			in.read(reinterpret_cast<char*>(&classType), sizeof(int));

			switch (classType)
			{
			// Obejct
			case 0: object = new CObject(); break;
			case 1: object = new CPlayer(); break;
			case 2: object = new CNPC(); break;
			// case 3: object = new CMonster(); break;
			// case 4: object = new CBullet(); break;
			// case 5: object = new CGun(); break;
			// case 6: object = new CSword(); break;
			case 3: object = new CObject(); break;
			case 4: object = new CObject(); break;
			case 5: object = new CObject(); break;
			case 6: object = new CObject(); break;
			case 7: object = new CBox(); break;
			case 8: object = new COnceItem(); break;
			}
		}
		else if (str == "<Name>")
		{
			string name;
			File::ReadStringFromFile(in, name);
		}
		else if (str == "<IsActive>")
		{
			int active;
			in.read(reinterpret_cast<char*>(&active), sizeof(int));
		}
		else if (str == "<Transform>")
		{
			XMFLOAT3 t[3] = {};

			in.read(reinterpret_cast<char*>(&t[0]), 3 * sizeof(XMFLOAT3));
		}
		else if (str == "<Mesh>")
		{
			File::ReadStringFromFile(in, str);
		}
		else if (str == "<Materials>")
		{
			int materialCount = 0;

			in.read(reinterpret_cast<char*>(&materialCount), sizeof(int));

			if (materialCount > 0)
			{
				// <Material>
				File::ReadStringFromFile(in, str);

				for (int i = 0; i < materialCount; ++i)
				{
					File::ReadStringFromFile(in, str);
				}
			}
		}
		else if (str == "<BoxCollider>")
		{
			XMFLOAT3 center = {};
			XMFLOAT3 extents = {};

			in.read(reinterpret_cast<char*>(&center), sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&extents), sizeof(XMFLOAT3));

			object->m_collider = true;
			object->m_origin.Center = center;
			object->m_origin.Extents = extents;
		}
		else if (str == "<MeshCollider>")
		{
			XMFLOAT3 center = {};
			XMFLOAT3 extents = {};

			in.read(reinterpret_cast<char*>(&center), sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&extents), sizeof(XMFLOAT3));
		}
		else if (str == "<ChildCount>")
		{
			int childCount = 0;

			in.read(reinterpret_cast<char*>(&childCount), sizeof(int));

			if (childCount > 0)
			{
				for (int i = 0; i < childCount; ++i)
				{
					CObject* child = CObject::LoadFrame(in);
				}
			}
		}
		else if (str == "</Frame>")
		{
			break;
		}
	}

	return object;
}
