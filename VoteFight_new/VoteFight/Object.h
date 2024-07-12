#pragma once

class CMesh;
class CMaterial;
class CComponent;
class CStateMachine;
class CRigidBody;
class CAnimator;
class CTransform;
class CSpriteRenderer;
class CCollider;
class CCamera;
class CScene;

class CObject
{
private:
	static UINT		    m_nextInstanceID;
	UINT			    m_instanceID;
	UINT				m_GroupType;

public:
	string              m_name;
	unsigned int		m_id{};
					    
	bool			    m_isActive;
	bool			    m_isDeleted;

	CMesh*			    m_mesh;
	vector<CMaterial*>  m_materials;

	vector<CComponent*> m_components;

	CObject*		    m_parent;
	vector<CObject*>    m_children;

	CObject();
	CObject(const CObject& rhs) = delete;
	virtual ~CObject();

	static CObject* Load(const string& fileName);

	UINT GetInstanceID();

	void SetName(const string& name);
	const string& GetName() const;

	void SetActive(bool isActive);
	bool IsActive();

	void SetDeleted(bool isDeleted);
	bool IsDeleted();

	void SetMesh(CMesh* mesh);
	CMesh* GetMesh();

	void SetGroupType(const UINT GroupType) { m_GroupType = GroupType; };
	UINT GetGroupType()const { return m_GroupType; };

	CMesh* GetMesh()const { return m_mesh; };
	vector<CMaterial*> GetMaterial() const { return m_materials; };
	
	void InTerrainSpace(const CScene& curScene);
	bool ChcekInTerrainSpace(const CScene& curScene);

	void AddMaterial(CMaterial* material);
	const vector<CMaterial*>& GetMaterials();

	virtual CComponent* CreateComponent(COMPONENT_TYPE componentType);
	CComponent* GetComponent(COMPONENT_TYPE componentType);

	CObject* GetParent();

	void AddChild(CObject* object);
	const vector<CObject*>& GetChildren();

	virtual void Init();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	CObject* FindFrame(const string& name);

	bool IsVisible(CCamera* camera);

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);

	virtual void Update();

	virtual void PreRender(CCamera* camera);
	virtual void Render(CCamera* camera);
	virtual void RenderBilboard(CCamera* camera) {}

	virtual const XMFLOAT3& GetPosition() ;
	virtual void SetPostion(const XMFLOAT3& rVector);

	virtual const XMFLOAT3& GetRotate();
	virtual void SetRotate(const XMFLOAT3& rRotate);

	virtual const XMFLOAT3& GetScale();
	virtual void SetScale(const XMFLOAT3& rScale);

	bool operator==(const CObject& other) const {
		return m_name == other.m_name;
	}

private:
	static CObject* LoadFrame(ifstream& in);
};

// 해시 함수 객체
struct CObjectPtrHash {
	std::size_t operator()(const CObject* obj) const {
		return std::hash<std::string>()(obj->GetName());
	}
};

// 비교 함수 객체
struct CObjectPtrEqual {
	bool operator()(const CObject* lhs, const CObject* rhs) const {
		return lhs->GetName() == rhs->GetName();
	}
};

