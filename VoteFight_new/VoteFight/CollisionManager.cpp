#include "pch.h"
#include "CollisionManager.h"

#include "SceneManager.h"

#include "Scene.h"

#include "Object.h"

#include "Transform.h"
#include "Collider.h"

CCollisionManager::CCollisionManager() :
	m_layerMask{},
	m_collisionStates()
{
}

CCollisionManager::~CCollisionManager()
{
}

void CCollisionManager::SetCollisionGroup(GROUP_TYPE group1, GROUP_TYPE group2)
{
	// �� ���� �׷� Ÿ���� ������, ū �׷� Ÿ���� ���� ����Ѵ�.
	UINT row = static_cast<UINT>(group1);
	UINT col = static_cast<UINT>(group2);

	if (row > col)
	{
		swap(row, col);
	}

	col = 1 << col;

	// �̹� üũ�Ǿ� �ִ� ���̾�ٸ�, üũ�� �����Ѵ�.
	if (m_layerMask[row] & col)
	{
		m_layerMask[row] &= ~col;
	}
	else
	{
		m_layerMask[row] |= col;
	}
}

void CCollisionManager::UpdateCollisionGroup(GROUP_TYPE group1, GROUP_TYPE group2)
{
	CScene* currentScene = CSceneManager::GetInstance()->GetCurrentScene();
	const vector<CObject*>& groupObject1 = currentScene->GetGroupObject(group1);
	const vector<CObject*>& groupObject2 = currentScene->GetGroupObject(group2);

	for (const auto& object1 : groupObject1)
	{
		for (const auto& object2 : groupObject2)
		{
			if (object1 == object2)
			{
				continue;
			}

			// �� ��ü�� �浹 ����
			CollisionID collisionID = {};

			collisionID.m_lowPart = object1->GetInstanceID();
			collisionID.m_highPart = object2->GetInstanceID();

			// ���� �����ӿ� �� ��ü�� �浹�� ���
			if (IsCollided(object1, object2))
			{
				// ���� �����ӿ��� �浹�� ���
				if (m_collisionStates[collisionID.m_quadPart])
				{
					// �� ��ü �� �� ��ü�� ��Ȱ��ȭ �Ǿ��ų�, ���� ������ ��ü�� ��� �� ��ü�� �浹�� ������Ų��.
					if ((!object1->IsActive()) || (object1->IsDeleted()) || (!object2->IsActive()) || (object2->IsDeleted()))
					{
						object1->OnCollisionExit(object2);
						object2->OnCollisionExit(object1);
						m_collisionStates[collisionID.m_quadPart] = false;
					}
					else
					{
						object1->OnCollision(object2);
						object2->OnCollision(object1);
					}
				}
				else
				{
					// �� ��ü �� �� ��ü�� ��Ȱ��ȭ �Ǿ��ų�, ���� ������ ��ü�� ��� �� ��ü�� �浹�� ��ȿȭ�ȴ�.
					if ((object1->IsActive()) && (!object1->IsDeleted()) && (object2->IsActive()) && (!object2->IsDeleted()))
					{
						object1->OnCollisionEnter(object2);
						object2->OnCollisionEnter(object1);
						m_collisionStates[collisionID.m_quadPart] = true;
					}
				}
			}
			else
			{
				// ���� �����ӿ� �浹�� ���
				if (m_collisionStates[collisionID.m_quadPart])
				{
					object1->OnCollisionExit(object2);
					object2->OnCollisionExit(object1);
					m_collisionStates[collisionID.m_quadPart] = false;
				}
			}
		}
	}
}

void CCollisionManager::ResetCollisionGroup()
{
	memset(m_layerMask, 0, sizeof(m_layerMask));
}

bool CCollisionManager::IsCollided(CObject* object1, CObject* object2)
{
	CCollider* collider1 = static_cast<CCollider*>(object1->GetComponent(COMPONENT_TYPE::COLLIDER));
	CCollider* collider2 = static_cast<CCollider*>(object2->GetComponent(COMPONENT_TYPE::COLLIDER));

	if ((collider1 == nullptr) || (collider2 == nullptr))
	{
		return false;
	}

	return collider1->GetBoundingBox().Intersects(collider2->GetBoundingBox());
}

void CCollisionManager::Update()
{
	for (UINT row = 0; row < static_cast<UINT>(GROUP_TYPE::COUNT); ++row)
	{
		for (UINT col = row; col < static_cast<UINT>(GROUP_TYPE::COUNT); ++col)
		{
			// ���̾� ����ũ�� üũ �Ǿ��ִ� �׷� ���� �浹�� �˻��Ѵ�.
			if (m_layerMask[row] & (1 << col))
			{
				UpdateCollisionGroup(static_cast<GROUP_TYPE>(row), static_cast<GROUP_TYPE>(col));
			}
		}
	}
}
