class CPlayer {
private:
	float	m_Xpos;
	float	m_Ypos;
	float	m_Zpos;

public:
	bool alive = true;
	bool ingame = false;

public:
	CPlayer();
	CPlayer(float _xPos, float _yPos, float _zPos);
	virtual ~CPlayer();

	float getXpos() { return m_Xpos; }
	float getYpos() { return m_Ypos; }
	float getZpos() { return m_Zpos; }

	void setXpos(float _x) { m_Xpos = _x; }
	void setYpos(float _y) { m_Xpos = _y; }
	void setZpos(float _z) { m_Xpos = _z; }
};