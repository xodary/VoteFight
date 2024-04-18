#include "pch.h"
#include "Object.h"
#include "AssetManager.h"
#include "Player.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "StateMachine.h"
#include "RigidBody.h"
#include "Collider.h"
#include "Animator.h"
#include "Transform.h"
#include "Camera.h"
#include "SkinnedMesh.h"
#include "Scene.h"

UINT CObject::m_nextInstanceID = 0;

CObject::CObject() :
	m_instanceID(m_nextInstanceID++),
	m_name(),
	m_isActive(),
	m_isDeleted(),
	m_mesh(),
	m_materials(),
	m_components(),
	m_parent(),
	m_children()
{
	m_components.resize(static_cast<size_t>(COMPONENT_TYPE::COUNT));

	// 모든 객체는 Transform 컴포넌트를 가진다.
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
}

CObject::~CObject()
{
	// AssetManager는 원본 머터리얼을 저장하고 있고, 모든 오브젝트는 해당 머터리얼을 복사한 인스턴스를 가지기 때문에, 이 클래스에서 소멸시켜주어야 한다.
	Utility::SafeDelete(m_materials);
	Utility::SafeDelete(m_components);
	Utility::SafeDelete(m_children);
}

CObject* CObject::Load(const string& fileName)
{
	CObject* object = {};
	string filePath = CAssetManager::GetInstance()->GetAssetPath() + "Model\\" + fileName + ".bin";
	ifstream in(filePath, ios::binary);
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Frames>")
		{
			cout << fileName << " 모델 로드 시작...\n";
			object = CObject::LoadFrame(in);
		}
		else if (str == "</Frames>")
		{
			cout << fileName << " 모델 로드 완료...\n";
			break;
		}
	}

	File::ReadStringFromFile(in, str);

	if (str == "<Animator>")
	{
		CAnimator* animator = static_cast<CAnimator*>(object->CreateComponent(COMPONENT_TYPE::ANIMATOR));

		animator->Load(in);
	}

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
			// Character
			case 0: object = new CObject(); break;
			case 1: object = new CPlayer(); break;
			}
		}
		else if (str == "<Name>")
		{
			File::ReadStringFromFile(in, object->m_name);
		}
		else if (str == "<IsActive>")
		{
			in.read(reinterpret_cast<char*>(&object->m_isActive), sizeof(int));
		}
		else if (str == "<Transform>")
		{
			// localPosition, localRotation, localScale
			XMFLOAT3 t[3] = {};

			in.read(reinterpret_cast<char*>(&t[0]), 3 * sizeof(XMFLOAT3));

			CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

			transform->SetLocalPosition(t[0]);
			transform->SetLocalRotation(t[1]);
			transform->SetLocalScale(t[2]);
		}
		else if (str == "<Mesh>")
		{
			File::ReadStringFromFile(in, str);
			
			CMesh* mesh = CAssetManager::GetInstance()->GetMesh(str);

			object->SetMesh(mesh);
		}
		else if (str == "<Materials>")
		{
			int materialCount = 0;

			in.read(reinterpret_cast<char*>(&materialCount), sizeof(int));

			if (materialCount > 0)
			{
				object->m_materials.reserve(materialCount);

				// <Material>
				File::ReadStringFromFile(in, str);

				for (int i = 0; i < materialCount; ++i)
				{
					File::ReadStringFromFile(in, str);
					
					// 머터리얼 인스턴스를 생성하고 추가한다.
					CMaterial* material = CAssetManager::GetInstance()->CreateMaterialInstance(str);

					object->AddMaterial(material);
				}
			}
		}
		else if (str == "<Collider>")
		{
			XMFLOAT3 center = {};
			XMFLOAT3 extents = {};

			in.read(reinterpret_cast<char*>(&center), sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&extents), sizeof(XMFLOAT3));

			// 메쉬를 가진 프레임은 콜라이더 컴포넌트를 생성한다.
			object->CreateComponent(COMPONENT_TYPE::COLLIDER);
			
			CCollider* collider = static_cast<CCollider*>(object->GetComponent(COMPONENT_TYPE::COLLIDER));

			collider->SetBoundingBox(center, extents);
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

					if (child != nullptr)
					{
						object->AddChild(child);
					}
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

UINT CObject::GetInstanceID()
{
	return m_instanceID;
}

void CObject::SetName(const string& name)
{
	m_name = name;
}

const string& CObject::GetName()
{
	return m_name;
}

void CObject::SetActive(bool isActive)
{
	m_isActive = isActive;
}

bool CObject::IsActive()
{
	return m_isActive;
}

void CObject::SetDeleted(bool isDeleted)
{
	m_isDeleted = isDeleted;
}

bool CObject::IsDeleted()
{
	return m_isDeleted;
}

void CObject::SetMesh(CMesh* mesh)
{
	m_mesh = mesh;
}

CMesh* CObject::GetMesh()
{
	return m_mesh;
}

void CObject::SetTerrainY(CScene* curScene)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 newVec = transform->GetPosition();
	newVec.y = curScene->GetTerrainHeight(newVec.x, newVec.z);
	transform->SetPosition(newVec);
}


void CObject::CheckInTerrainSpace(const CScene& curScene) 
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 curVec = transform->GetPosition();
	if (curVec.x < 0)
		curVec.x = 0;
	else if (curVec.x > curScene.GetTerrain()->GetWidth())
		curVec.x = curScene.GetTerrain()->GetWidth();

	if (curVec.z < 0)
		curVec.z = 0;
	else if (curVec.z > curScene.GetTerrain()->GetLength())
		curVec.z = curScene.GetTerrain()->GetLength();

	transform->SetPosition(curVec);
}

void CObject::AddMaterial(CMaterial* material)
{
	if (material != nullptr)
	{
		m_materials.push_back(material);
	}
}

const vector<CMaterial*>& CObject::GetMaterials()
{
	return m_materials;
}

CComponent* CObject::CreateComponent(COMPONENT_TYPE componentType)
{
	// 이미 해당 컴포넌트를 가지고 있었다면, 삭제 후 새로 생성한다.
	if (m_components[static_cast<int>(componentType)] != nullptr)
	{
		delete m_components[static_cast<int>(componentType)];
		m_components[static_cast<int>(componentType)] = nullptr;
	}

	switch (componentType)
	{
	case COMPONENT_TYPE::STATE_MACHINE:
		m_components[static_cast<int>(componentType)] = new CStateMachine();
		break;
	case COMPONENT_TYPE::RIGIDBODY:
		m_components[static_cast<int>(componentType)] = new CRigidBody();
		break;
	case COMPONENT_TYPE::ANIMATOR:
		m_components[static_cast<int>(componentType)] = new CSkinningAnimator();
		break;
	case COMPONENT_TYPE::TRANSFORM:
		m_components[static_cast<int>(componentType)] = new CTransform();
		break;
	case COMPONENT_TYPE::COLLIDER:
		m_components[static_cast<int>(componentType)] = new CCollider();
		break;
	}

	m_components[static_cast<int>(componentType)]->SetOwner(this);

	return m_components[static_cast<int>(componentType)];
}

CComponent* CObject::GetComponent(COMPONENT_TYPE componentType)
{
	return m_components[static_cast<int>(componentType)];
}

CObject* CObject::GetParent()
{
	return m_parent;
}

void CObject::AddChild(CObject* object)
{
	if (object != nullptr)
	{
		m_children.push_back(object);
		object->m_parent = this;
	}
}

const vector<CObject*>& CObject::GetChildren()
{
	return m_children;
}

void CObject::Init()
{
}

void CObject::CreateShaderVariables()
{
}

void CObject::UpdateShaderVariables()
{
	for (const auto& component : m_components)
	{
		if (component != nullptr)
		{
			component->UpdateShaderVariables();
		}
	}
}

void CObject::ReleaseShaderVariables()
{
}

void CObject::ReleaseUploadBuffers()
{
}

CObject* CObject::FindFrame(const string& name)
{
	CObject* object = nullptr;

	if (m_name == name)
	{
		return this;
	}

	for (const auto& child : m_children)
	{
		object = child->FindFrame(name);

		if (object != nullptr)
		{
			break;
		}
	}

	return object;
}

bool CObject::IsVisible(CCamera* camera)
{
	return true;
}

void CObject::OnCollisionEnter(CObject* collidedObject)
{
	cout << "OnCollisionEnter()\n";
}

void CObject::OnCollision(CObject* collidedObject)
{
}

void CObject::OnCollisionExit(CObject* collidedObject)
{
	cout << "OnCollisionExit()\n";
}

void CObject::Update()
{
	for (const auto& component : m_components)
	{
		if (component != nullptr)
		{
			component->Update();
		}
	}

	for (const auto& child : m_children)
	{
		if ((child->m_isActive) && (!child->m_isDeleted))
		{
			child->Update();
		}
	}
}

void CObject::PreRender(CCamera* camera)
{
	UpdateShaderVariables();

	// DepthWrite의 경우, 직교 투영변환 행렬을 사용하기 때문에 프러스텀 컬링을 수행할 수 없다.
	if (m_mesh != nullptr)
	{
		for (int i = 0; i < m_materials.size(); ++i)
		{
		//	m_materials[i]->SetPipelineState(RENDER_TYPE::DEPTH_WRITE);
			m_mesh->Render(i);
		}
	}

	for (const auto& child : m_children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->PreRender(camera);
		}
	}
}

void CObject::Render(CCamera* camera)
{
	UpdateShaderVariables();

	if (IsVisible(camera))
	{
		if (m_mesh != nullptr)
		{
			for (int i = 0; i < m_materials.size(); ++i)
			{
				m_materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
				m_materials[i]->UpdateShaderVariables();
				m_mesh->Render(i);
			}
		}
	}

	// [Debug] Render BoundingBox
	CComponent* collider = GetComponent(COMPONENT_TYPE::COLLIDER);

	if (collider != nullptr)
	{
		collider->Render(camera);
	}

	for (const auto& child : m_children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}

}
