#include "pch.h"
#include "Scene.h"
#include "GameFramework.h"
#include "AssetManager.h"
#include "CameraManager.h"
#include "Player.h"
#include "Animator.h"
#include "UI.h"
#include "Transform.h"
#include "Camera.h"
#include "Bullet.h"

CScene::CScene() :
	m_name(),
	m_objects()
{
}

CScene::~CScene()
{
	ReleaseShaderVariables();

	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		Utility::SafeDelete(m_objects[i]);
	}
}

void CScene::Load(const string& fileName)
{
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Scene\\" + fileName;
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
			case GROUP_TYPE::PLAYER:
			case GROUP_TYPE::MONSTER:
				for (int i = 0; i < instanceCount; ++i)
				{
					CObject* object = CObject::Load(modelFileName);
					CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

					object->SetActive(isActives[i]);

					transform->SetPosition(transforms[3 * i]);
					transform->SetRotation(transforms[3 * i + 1]);
					transform->SetScale(transforms[3 * i + 2]);

					transform->Update();
					transform->Update();
					object->Init();

					AddObject(groupType, object);
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

void CScene::CreateShaderVariables()
{
}

void CScene::UpdateShaderVariables()
{
}

void CScene::ReleaseShaderVariables()
{
}

void CScene::SetName(const string& name)
{
	m_name = name;
}

const string& CScene::GetName()
{
	return m_name;
}

void CScene::AddObject(const GROUP_TYPE& groupType, CObject* object)
{
	object->SetGroupType((UINT)GROUP_TYPE(groupType));
	object->m_id = m_objects[static_cast<int>(groupType)].size();

	if (object != nullptr)
	{
		m_objects[static_cast<int>(groupType)][object->m_id] = object;
	}

	// 클라이언트 시야처리 (SECTOR)
	CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 pos = transform->GetPosition();
	int xNewCell = clamp((int)(pos.x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
	int zNewCell = clamp((int)(pos.z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);
	ObjectListSector[zNewCell * SECTOR_RANGE_ROW + xNewCell].insert(object);
}

void CScene::AddObject(const GROUP_TYPE& groupType, CObject* object, UINT id)
{
	object->SetGroupType((UINT)GROUP_TYPE(groupType));
	object->m_id = id;

	if (object != nullptr)
	{
		m_objects[static_cast<int>(groupType)][id] = object;
	}

	// 클라이언트 시야처리 (SECTOR)
	CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 pos = transform->GetPosition();
	int xNewCell = clamp((int)(pos.x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
	int zNewCell = clamp((int)(pos.z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);
	ObjectListSector[zNewCell * SECTOR_RANGE_ROW + xNewCell].insert(object);
}

const unordered_map<int, CObject*>& CScene::GetGroupObject(GROUP_TYPE groupType)
{
	return m_objects[static_cast<int>(groupType)];
}

CObject* CScene::GetIDObject(GROUP_TYPE groupType, int id)
{
	if (m_objects[static_cast<int>(groupType)].count(id) == 0) return NULL;
	return m_objects[static_cast<int>(groupType)][id];
}

void CScene::DeleteGroupObject(GROUP_TYPE groupType)
{
	Utility::SafeDelete(m_objects[static_cast<int>(groupType)]);
}

void CScene::DeleteObject(GROUP_TYPE groupType, UINT id)
{
	CObject* object = m_objects[static_cast<int>(groupType)][id];

	CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 pos = transform->GetPosition();
	int xNewCell = clamp((int)(pos.x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
	int zNewCell = clamp((int)(pos.z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);
	ObjectListSector[zNewCell * SECTOR_RANGE_ROW + xNewCell].erase(object);

	m_objects[static_cast<int>(groupType)].erase(id);
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			object.second->ReleaseUploadBuffers();
		}
	}
}

void CScene::Update()
{
	for (auto& object : GetViewList(0))
	{
		if ((object->IsActive()) && (!object->IsDeleted()))
		{
			object->Update();
		}
	}
	for (auto& object : m_objects[(int)GROUP_TYPE::UI])
	{
		if ((object.second->IsActive()) && (!object.second->IsDeleted()))
		{
			object.second->Update();
		}
	}
}

void CScene::PreRender()
{
	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();

	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (int i = 0; i < static_cast<int>(GROUP_TYPE::UI); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			if ((object.second->IsActive()) && (!object.second->IsDeleted()))
			{
				object.second->PreRender(camera);
			}
		}
	}

}

void CScene::Render()
{
}

void CScene::PostRender()
{
}

void CScene::RenderImGui()
{
}

unordered_set<CObject*> CScene::GetViewList(int stateNum)
{
	unordered_set<CObject*> newlist;

	CObject* myPlayer = GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id);
	newlist.insert(myPlayer);
	CTransform* transform = reinterpret_cast<CTransform*>(myPlayer->GetComponent(COMPONENT_TYPE::TRANSFORM));

	int xNewCell = clamp((int)(transform->GetPosition().x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
	int zNewCell = clamp((int)(transform->GetPosition().z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);

	switch (stateNum)
	{
	case 0:		//	Main
	{
		short searchDirection_[16][2] = { {0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
			{-2, -1},{-2, 0},{-2, 1},{-2, 2}, {1, 2},{0, 2},{-1, 2} };
		for (int i = 0; i < 16; ++i) {
			short xSearchCell = clamp(xNewCell + searchDirection_[i][0], 0, SECTOR_RANGE_COL - 1);
			short zSearchCell = clamp(zNewCell + searchDirection_[i][1], 0, SECTOR_RANGE_ROW - 1);
			for (auto& object : ObjectListSector[zSearchCell * SECTOR_RANGE_ROW + xSearchCell]) {
				if (object->GetGroupType() != (int)GROUP_TYPE::UI) newlist.insert(object);
			}
		}
	}
	break;
	case 1:		// Depth Write
	{
		short searchDirection_[9][2] = { {0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1} };
		for (int i = 0; i < 9; ++i) {
			short xSearchCell = clamp(xNewCell + searchDirection_[i][0], 0, SECTOR_RANGE_COL - 1);
			short zSearchCell = clamp(zNewCell + searchDirection_[i][1], 0, SECTOR_RANGE_ROW - 1);
			for (auto& object : ObjectListSector[zSearchCell * SECTOR_RANGE_ROW + xSearchCell]) {
				if (object->GetGroupType() != (int)GROUP_TYPE::UI) newlist.insert(object);
			}
		}
	}
	break;
	}
	oldXCell = xNewCell;
	oldZCell = zNewCell;
	my_vl = newlist;
	return newlist;
}
