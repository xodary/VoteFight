#include "pch.h"
#include "Component.h"

CComponent::CComponent() :
	m_isEnabled(true),
	m_owner()
{
}

CComponent::~CComponent()
{
}

void CComponent::SetEnabled(bool isEnabled)
{
	m_isEnabled = isEnabled;
}

bool CComponent::IsEnabled()
{
	return m_isEnabled;
}

void CComponent::SetOwner(CObject* owner)
{
	m_owner = owner;
}

CObject* CComponent::GetOwner()
{
	return m_owner;
}

void CComponent::CreateShaderVariables()
{
}

void CComponent::UpdateShaderVariables()
{
}

void CComponent::ReleaseShaderVariables()
{
}

void CComponent::Render(CCamera* camera)
{
}
