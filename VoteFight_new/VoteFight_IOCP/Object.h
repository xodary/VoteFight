#pragma once

class CObject
{
public:
	XMFLOAT3	m_Pos;
	XMFLOAT3	m_Rota;
	XMFLOAT3	m_Sca;
	float		m_Angle;
	float		m_Velocity;
	XMFLOAT3	m_Vec;
	int			m_id;
	int			m_grouptype;
	std::string	m_modelname;

	bool alive = true;
	bool ingame = false;

	CObject() {}
	~CObject() {}
};

