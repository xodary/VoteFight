class CPlayer {
private:
	// float		m_Xpos;
	// float		m_Ypos;
	// float		m_Zpos;

public:
	XMFLOAT3	m_Pos;
	bool alive = true;
	bool ingame = false;

public:
	CPlayer();
	CPlayer(XMFLOAT3 _pos);
	virtual ~CPlayer();

	XMFLOAT3 getPos() { return m_Pos; }

	// float getXpos() { return m_Xpos; }
	// float getYpos() { return m_Ypos; }
	// float getZpos() { return m_Zpos; }

	// void setXpos(float _x) { m_Xpos = _x; }
	// void setYpos(float _y) { m_Xpos = _y; }
	//void setZpos(float _z) { m_Xpos = _z; }
};