#pragma once
#include "Component.h"

class CTransform : public CComponent
{
public:
	static const XMFLOAT3 m_worldRight;
	static const XMFLOAT3 m_worldUp;
	static const XMFLOAT3 m_worldForward;

protected:
	XMFLOAT3              m_position;
	XMFLOAT3              m_rotation;
	XMFLOAT3              m_scale;

	XMFLOAT3              m_localPosition;
	XMFLOAT3			  m_localRotation;
	XMFLOAT3			  m_localScale;

	XMFLOAT4X4			  m_worldMatrix;

public:

	CTransform();
	virtual ~CTransform();

	void SetPosition(const XMFLOAT3& position);
	const XMFLOAT3& GetPosition();

	void SetRotation(const XMFLOAT3& rotation);
	const XMFLOAT3& GetRotation();

	void SetScale(const XMFLOAT3& scale);
	const XMFLOAT3& GetScale();

	void SetLocalPosition(const XMFLOAT3& localPosition);
	const XMFLOAT3& GetLocalPosition();

	void SetLocalRotation(const XMFLOAT3& localRotation);
	const XMFLOAT3& GetLocalRotation();

	void SetLocalScale(const XMFLOAT3& localScale);
	const XMFLOAT3& GetLocalScale();

	const XMFLOAT4X4& GetWorldMatrix();

	XMFLOAT3 GetRight();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetForward();

	virtual void UpdateShaderVariables();

	void Translate(const XMFLOAT3& shift);
	
	void Rotate(const XMFLOAT3& rotation);
	void LookTo(const XMFLOAT3& direction);
	void AddScale(const XMFLOAT3& scale);
	void MultiplyScale(const XMFLOAT3& scale);

	virtual void Update();
};

//=========================================================================================================================

class CRectTransform : public CTransform
{
private:
	XMFLOAT2 m_rect; // width, height

public:
	CRectTransform();
	virtual ~CRectTransform();

	void SetRect(const XMFLOAT2& rect);
	const XMFLOAT2& GetRect();

	virtual void UpdateShaderVariables();
};
