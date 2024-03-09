#include "pch.h"
#include "Asset.h"

CAsset::CAsset() :
    m_name()
{
}

CAsset::~CAsset()
{
}

void CAsset::SetName(const string& name)
{
    m_name = name;
}

const string& CAsset::GetName()
{
    return m_name;
}

void CAsset::CreateShaderVariables()
{
}

void CAsset::UpdateShaderVariables()
{
}

void CAsset::ReleaseShaderVariables()
{
}

void CAsset::ReleaseUploadBuffers()
{
}
