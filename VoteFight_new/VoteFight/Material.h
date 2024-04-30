#pragma once
#include "Asset.h"

class CTexture;
class CShader;

class CMaterial : public CAsset
{
	friend class CAssetManager;

private:
	XMFLOAT4		  m_color;

	int				  m_textureMask;
	XMFLOAT2		  m_textureScale;
	vector<CTexture*> m_textures;

	vector<CShader*>  m_shaders;
	int	              m_stateNum;

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

private:
	CMaterial();

public:
	virtual ~CMaterial();

	void SetColor(const XMFLOAT4& color);
	const XMFLOAT4& GetColor();

	void SetStateNum(int stateNum);
	int GetStateNum();

	void SetTexture(CTexture* texture);
	const vector<CTexture*>& GetTextures();

	void AddShader(CShader* shader);
	const vector<CShader*>& GetShaders();

	void Load(ifstream& in);

	virtual void UpdateShaderVariables();

	void SetPipelineState(RENDER_TYPE renderType);
};
