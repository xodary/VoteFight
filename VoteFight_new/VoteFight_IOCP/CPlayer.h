#include "../VoteFight/InputManager.h"
#include "Object.h"

//class CStateMachine;

class CPlayer : public CObject 
{
public:
	int		m_tickets;
	int		m_Weapon;

	CPlayer();
	CPlayer(XMFLOAT3 _pos);
	virtual ~CPlayer();

};