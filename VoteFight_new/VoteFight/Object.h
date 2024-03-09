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

class CObject
{
private:
	static UINT		    m_nextInstanceID;
	UINT			    m_instanceID;
		
protected:
	string              m_name;
					    
	bool			    m_isActive;
	bool			    m_isDeleted;

	CMesh*			    m_mesh;
	vector<CMaterial*>  m_materials;

	vector<CComponent*> m_components;

	CObject*		    m_parent;
	vector<CObject*>    m_children;

public:
	CObject();
	CObject(const CObject& rhs) = delete;
	virtual ~CObject();

	static CObject* Load(const string& fileName);

	UINT GetInstanceID();

	void SetName(const string& name);
	const string& GetName();

	void SetActive(bool isActive);
	bool IsActive();

	void SetDeleted(bool isDeleted);
	bool IsDeleted();

	void SetMesh(CMesh* mesh);
	CMesh* GetMesh();

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

	CObject* CheckRayIntersection(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float& hitDistance, float maxDistance);

	bool IsVisible(CCamera* camera);

	virtual void OnCollisionEnter(CObject* collidedObject);
	virtual void OnCollision(CObject* collidedObject);
	virtual void OnCollisionExit(CObject* collidedObject);

	virtual void Update();

	virtual void PreRender(CCamera* camera);
	virtual void Render(CCamera* camera);

private:
	static CObject* LoadFrame(ifstream& in);
};
