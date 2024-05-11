#include "CCharacter.h"

class CPlayer : public CCharacter {
private:
	float	m_Xpos = 5;
	float	m_Ypos = 0;
	float	m_Zpos = 5;

public:
	bool alive = true;
	bool ingame = false;

public:
	CPlayer();
	virtual ~CPlayer();

	float getXpos(){ return m_Xpos; }
	float getYpos() { return m_Ypos; }
	float getZpos() { return m_Zpos; }

	void setXpos(float x) { m_Xpos = x; }
	void setYpos(float y) { m_Xpos = y; }
	void setZpos(float z) { m_Xpos = z; }
};

