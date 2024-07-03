#include "pch.h"
#include "Object.h"
#include "AssetManager.h"
#include "InputManager.h"

#include "Player.h"
#include "NPC.h"
#include "Monster.h"
#include "Bullet.h"

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
#include "Bullet.h"
#include "Weapon.h"

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
	SetGroupType((UINT)GROUP_TYPE::STRUCTURE);

	// ���?��ü�� Transform ������Ʈ�� ������.
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
}

CObject::~CObject()
{
	// AssetManager�� ���� ���͸����� �����ϰ� �ְ�, ���?������Ʈ�� �ش� ���͸����� ������ �ν��Ͻ��� ������ ������, �� Ŭ�������� �Ҹ�����־�� �Ѵ�.
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
			cout << fileName << " �� �ε� ����...\n";
			object = CObject::LoadFrame(in);
		}
		else if (str == "</Frames>")
		{
			cout << fileName << " �� �ε� �Ϸ�...\n";
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
			// Obejct
			case 0: object = new CObject(); break;
			case 1: object = new CPlayer(); break;
			case 2: object = new CNPC(); break;
			case 3: object = new CMonster(); break;
			case 4: object = new CBullet(); break;
			case 5: object = new CGun(); break;
			case 6: object = new CSword(); break;
			}
		}
		else if (str == "<Name>")
		{
			File::ReadStringFromFile(in, object->m_name);
			cout << "�� �̸��� : " << object->m_name << endl;
		}
		else if (str == "<IsActive>")
		{
			in.read(reinterpret_cast<char*>(&object->m_isActive), sizeof(int));
		}
		else if (str == "<Transform>")
		{
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
					
					// ���͸��� �ν��Ͻ��� �����ϰ� �߰��Ѵ�.
					CMaterial* material = CAssetManager::GetInstance()->CreateMaterialInstance(str);

					object->AddMaterial(material);
				}
			}
		}
		else if (str == "<BoxCollider>")
		{
			XMFLOAT3 center = {};
			XMFLOAT3 extents = {};

			in.read(reinterpret_cast<char*>(&center), sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&extents), sizeof(XMFLOAT3));

			// �޽��� ���� �������� �ݶ��̴� ������Ʈ�� �����Ѵ�.
			if (object->GetComponent(COMPONENT_TYPE::COLLIDER) == NULL)
				object->CreateComponent(COMPONENT_TYPE::COLLIDER);

			CCollider* collider = static_cast<CCollider*>(object->GetComponent(COMPONENT_TYPE::COLLIDER));

			collider->SetBoundingBox(center, extents);
		}
		else if (str == "<MeshCollider>")
		{
			XMFLOAT3 center = {};
			XMFLOAT3 extents = {};

			in.read(reinterpret_cast<char*>(&center), sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&extents), sizeof(XMFLOAT3));

			// �޽��� ���� �������� �ݶ��̴� ������Ʈ�� �����Ѵ�.
			if (object->GetComponent(COMPONENT_TYPE::COLLIDER) == NULL)
				object->CreateComponent(COMPONENT_TYPE::COLLIDER);

			CCollider* collider = static_cast<CCollider*>(object->GetComponent(COMPONENT_TYPE::COLLIDER));

			collider->SetMeshBoundingBox(center, extents);
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

const string& CObject::GetName() const
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

void CObject::InTerrainSpace(const CScene& curScene)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 curVec = transform->GetPosition();
	float velocity = 0.001f;
	if (curVec.x < 0)
		curVec.x = velocity;
	else if (curVec.x > curScene.GetTerrain()->m_nWidth)
		curVec.x = curScene.GetTerrain()->m_nWidth - velocity;

	if (curVec.z < 0)
		curVec.z = velocity;
	else if (curVec.z > curScene.GetTerrain()->m_nLength)
		curVec.z = curScene.GetTerrain()->m_nLength - velocity;

	transform->SetPosition(curVec);
}

bool CObject::ChcekInTerrainSpace(const CScene& curScene)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	XMFLOAT3 curVec = transform->GetPosition();
	if (curVec.x <= 0)
		return false;
	else if (curVec.x >= curScene.GetTerrain()->m_nWidth)
		return false;

	if (curVec.z <= 0)
		return false;
	else if (curVec.z >= curScene.GetTerrain()->m_nLength)
		return false;

	return true;
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
	// �̹� �ش� ������Ʈ�� ������ �־��ٸ�, ���� �� ���� �����Ѵ�.
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
	CCollider* collider = static_cast<CCollider*>(m_components[static_cast<int>(COMPONENT_TYPE::COLLIDER)]);

	if ((camera != nullptr) && (collider != nullptr) && (collider->m_meshcollider))
	{
		return camera->IsInBoundingFrustum(collider->GetMeshBoundingBox());
	}

	return false;
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

	// DepthWrite�� ���? ���� ������ȯ �����?����ϱ�?������ �������� �ø��� ������ �� ����.
	if (m_mesh != nullptr)
	{
		for (int i = 0; i < m_materials.size(); ++i)
		{
			m_materials[i]->SetPipelineState(RENDER_TYPE::DEPTH_WRITE);
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

	if (KEY_HOLD(KEY::B)) 
	{
		// [Debug] Render BoundingBox
		CComponent* collider = GetComponent(COMPONENT_TYPE::COLLIDER);

		if (collider != nullptr)
		{
			collider->Render(camera);
		}
	}

	for (const auto& child : m_children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}

const XMFLOAT3& CObject::GetPostion() 
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	return transform->GetPosition();
}

void CObject::SetPostion(const XMFLOAT3& rVector)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	transform->SetPosition(rVector);
}

const XMFLOAT3& CObject::GetRotate()
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	return transform->GetRotation();

}

void CObject::SetRotate(const XMFLOAT3& rRotate)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	transform->SetRotation(rRotate);
}

const XMFLOAT3& CObject::GetScale()
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	return transform->GetScale();
}

void CObject::SetScale(const XMFLOAT3& rScale)
{
	CTransform* transform = static_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	transform->SetScale(rScale);
}
