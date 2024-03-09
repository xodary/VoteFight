#include "pch.h"
#include "Scene.h"

#include "GameFramework.h"

#include "AssetManager.h"
#include "CameraManager.h"

#include "Player.h"
#include "Guard.h"
#include "UI.h"
#include "Trigger.h"

#include "Animator.h"
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
			case GROUP_TYPE::TERRAIN:
			case GROUP_TYPE::STRUCTURE:
			case GROUP_TYPE::PLAYER:
				for (int i = 0; i < instanceCount; ++i)
				{
					CObject* object = CObject::Load(modelFileName);
					CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

					object->SetActive(isActives[i]);
					transform->SetPosition(transforms[3 * i]);
					transform->SetRotation(transforms[3 * i + 1]);
					transform->SetScale(transforms[3 * i + 2]);
					transform->Update();
					object->Init();

					AddObject(groupType, object);
				}
				break;
			case GROUP_TYPE::TRIGGER:
			{
				// <RootObject>
				string rootObjectName;
				vector<string> targetObjectNames;

				File::ReadStringFromFile(in, str);
				File::ReadStringFromFile(in, rootObjectName);

				if (rootObjectName != "None")
				{
					// <TargetObjects>
					File::ReadStringFromFile(in, str);

					int targetCount = 0;

					in.read(reinterpret_cast<char*>(&targetCount), sizeof(int));
					targetObjectNames.resize(targetCount);

					for (int i = 0; i < targetCount; ++i)
					{
						File::ReadStringFromFile(in, targetObjectNames[i]);
					}
				}

				const vector<CObject*>& structures = GetGroupObject(GROUP_TYPE::STRUCTURE);

				for (int i = 0; i < instanceCount; ++i)
				{
					CTrigger* trigger = static_cast<CTrigger*>(CObject::Load(modelFileName));
					CTransform* transform = static_cast<CTransform*>(trigger->GetComponent(COMPONENT_TYPE::TRANSFORM));

					trigger->SetActive(isActives[i]);
					transform->SetPosition(transforms[3 * i]);
					transform->SetRotation(transforms[3 * i + 1]);
					transform->SetScale(transforms[3 * i + 2]);
					transform->Update();
					trigger->Init();

					for (const auto& structure : structures)
					{
						// Ÿ�� ������Ʈ�� �ֻ��� �������� ã�´�.
						CObject* rootObject = structure->FindFrame(rootObjectName);

						if (rootObject != nullptr)
						{
							for (const auto& targetObjectName : targetObjectNames)
							{
								// �ֻ��� �����ӿ��� Ÿ�� ������Ʈ�� ã�� Ʈ���ſ� �߰��Ѵ�.
								CObject* targetObject = rootObject->FindFrame(targetObjectName);

								trigger->AddTargetObject(targetObject);
							}

							break;
						}
					}

					AddObject(groupType, trigger);
				}
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

void CScene::AddObject(GROUP_TYPE groupType, CObject* object)
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
			}
		}
	}
}

void CScene::PreRender()
{
}

void CScene::Render()
{
	CCamera* camera = CCameraManager::GetInstance()->GetMainCamera();

	camera->RSSetViewportsAndScissorRects();
	camera->UpdateShaderVariables();

	for (int i = 0; i <= static_cast<int>(GROUP_TYPE::BILBOARD); ++i)
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
