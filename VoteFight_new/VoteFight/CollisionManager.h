#pragma once

class CObject;

union CollisionID
{
	struct
	{
		UINT m_lowPart;
		UINT m_highPart;
	};

	UINT64 m_quadPart;
};

class CCollisionManager : public CSingleton<CCollisionManager>
{
	friend class CSingleton;

private:
	UINT						m_layerMask[static_cast<int>(GROUP_TYPE::COUNT)]; // 그룹 간의 충돌체크 여부
	unordered_map<UINT64, bool> m_collisionStates;								  // 객체 간의 충돌 상태

private:
	CCollisionManager();
	virtual ~CCollisionManager();

	void UpdateCollisionGroup(GROUP_TYPE group1, GROUP_TYPE group2);
	bool IsCollided(CObject* object1, CObject* object2);

public:
	void SetCollisionGroup(GROUP_TYPE group1, GROUP_TYPE group2);
	void ResetCollisionGroup();

	void Update();
};
