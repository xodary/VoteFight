#pragma once

class CObject;
class CCamera;

class CComponent abstract
{
protected:
	bool     m_isEnabled;

	CObject* m_owner;

public:
	CComponent();
	CComponent(const CComponent& rhs) = delete;
	virtual ~CComponent();

	CComponent& operator =(const CComponent& rhs) = delete;

	void SetEnabled(bool isEnabled);
	bool IsEnabled();

	void SetOwner(CObject* owner);
	CObject* GetOwner();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void Update() = 0;
	virtual void Render(CCamera* camera);
};
