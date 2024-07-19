#include "../VoteFight/InputManager.h"
#include "Object.h"

//class CStateMachine;

class CPlayer : public CObject 
{
public:
	int		m_Health;
	//int		m_weapon;

	CPlayer();
	CPlayer(XMFLOAT3 _pos);
	virtual ~CPlayer();

};