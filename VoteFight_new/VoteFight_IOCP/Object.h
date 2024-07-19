#pragma once

class CObject
{
public:
	XMFLOAT3	m_Pos;
	XMFLOAT3	m_Rota;
	XMFLOAT3	m_Sca;
	float		m_Angle;
	float		m_Look;
	float		m_Velocity;
	XMFLOAT3	m_Vec;
	int			m_id;
	int			m_grouptype;
	std::string	m_modelname;
	std::string	m_upAnimation;

	bool		m_collider = false;
	BoundingBox	m_origin;
	BoundingBox m_boundingBox;

	chrono::system_clock::time_point		m_AnilastTime;

	bool alive = true;
	bool ingame = false;

	CObject();
	~CObject() {}

	static CObject* Load(const string& fileName);
	static CObject* LoadFrame(ifstream& in);
};

