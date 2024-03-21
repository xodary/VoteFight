#pragma once
#include "Scene.h"

struct Fog
{
	XMFLOAT4 m_color;
	float	 m_density;
};

struct LIGHT
{
	bool	   m_isActive;
			   
	XMFLOAT3   m_position;
	XMFLOAT3   m_direction;
			   
	int		   m_type;
			   
	XMFLOAT4   m_color;
			   
	XMFLOAT3   m_attenuation;
	float 	   m_fallOff;
	float	   m_range;
	float 	   m_theta;
	float	   m_phi;
			   
	bool	   m_shadowMapping;
	XMFLOAT4X4 m_toTexCoord;
};

struct CB_GameScene
{
	LIGHT m_lights[MAX_LIGHTS];
	Fog   m_fog;
};

class CGameScene : public CScene
{
	friend class CSceneManager;

private:
	ComPtr<ID3D12Resource> m_d3d12GameScene;
	CB_GameScene*		   m_mappedGameScene;

	CObject*			   m_towerLight;
	float				   m_towerLightAngle;

	//INIT_GAME_DATA		   m_InitGameData{};

private:
	CGameScene();

	virtual void Enter();
	virtual void Exit();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

public:
	virtual ~CGameScene();

	virtual void Init();

	virtual void Update();

	virtual void PreRender();
	virtual void Render();
};
