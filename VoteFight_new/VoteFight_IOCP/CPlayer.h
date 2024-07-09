#include "../VoteFight/InputManager.h"
#include "Object.h"

//class CStateMachine;

class CPlayer : CObject {
public:
	XMFLOAT3	m_Pos;
	float		m_Rota;
	float		m_Velocity;
	XMFLOAT3	m_Vec;
	unordered_map<KEY, bool> m_Keys;
	//CStateMachine* state;
	
	bool alive = true;
	bool ingame = false;

	CPlayer();
	CPlayer(XMFLOAT3 _pos);
	virtual ~CPlayer();

};