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
	if (!in)
	{
		cout << "파일 : " << filePath << "을 찾지 못함" << endl;
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
				for (int i = 0; i < instanceCount; ++i)
				{
					CObject* object = CObject::Load(modelFileName);
					CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

					object->SetActive(isActives[i]);
					object->SetGroupType((UINT)GROUP_TYPE(groupType));

					XMFLOAT3 currPosition = transforms[3 * i];
					currPosition.y = GetTerrainHeight(currPosition.x, currPosition.z);
					cout << fileName << endl;
					transform->SetPosition(currPosition);
					//transform->SetPosition(XMFLOAT3(currPosition.x, GetTerrainHeight(currPosition.x, currPosition.z), currPosition.z));
					transform->SetRotation(transforms[3 * i + 1]);
					transform->SetScale(transforms[3 * i + 2]);
					transform->Update();
					object->Init();

					AddObject(groupType, object);
				}
				break;
			case GROUP_TYPE::PLAYER:
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

void CScene::LoadUI(const string& fileName)
{
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Scene\\" + fileName;
	ifstream in(filePath, ios::binary);
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<UI>")
		{
			CUI* ui = CUI::Load(in);
			ui->SetGroupType((UINT)GROUP_TYPE::UI);
			AddObject(GROUP_TYPE::UI, ui);
		}
		else if (str == "</UIs>")
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
	if (object != nullptr)
	{
		m_objects[static_cast<int>(groupType)].push_back(object);
	}
}

const vector<CObject*>& CScene::GetGroupObject(GROUP_TYPE groupType)
{
	return m_objects[static_cast<int>(groupType)];
}

void CScene::DeleteGroupObject(GROUP_TYPE groupType)
{
	Utility::SafeDelete(m_objects[static_cast<int>(groupType)]);
}

void CScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			object->ReleaseUploadBuffers();
		}
	}
}

void CScene::Update()
{
	for (int i = 0; i < static_cast<int>(GROUP_TYPE::COUNT); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			if ((object->IsActive()) && (!object->IsDeleted()))
			{
				object->Update();
				if (i == static_cast<int>(GROUP_TYPE::UI)) continue;
				if (m_terrain && (object->GetInstanceID() != (UINT)GROUP_TYPE::UI ))object->InTerrainSpace(*this);
		
			}
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
			if ((object->IsActive()) && (!object->IsDeleted()))
			{
		//		object->PreRender(camera);
			}
		}
	}

}

void CScene::Render()
{
	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();

	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	if (m_terrain) m_terrain->Render(camera);

	for (int i = 0; i < static_cast<int>(GROUP_TYPE::UI); ++i)
	{
		for (const auto& object : m_objects[i])
		{
			if ((object->IsActive()) && (!object->IsDeleted()))
			{
				object->Render(camera);
			}
		}
	}

	camera = CCameraManager::GetInstance()->GetUICamera();
	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (const auto& object : m_objects[static_cast<int>(GROUP_TYPE::UI)])
	{
		if ((object->IsActive()) && (!object->IsDeleted()))
		{
			object->Render(camera);
		}
	}

}

void CScene::PostRender()
{
}
