#pragma once
#include "Character.h"

class CQuest;
class C_NPC_Item;
class CBilboardUI;

class CNPC : public CCharacter
{
private:
	string			m_spineName;
public:
	vector<string>	m_needs;
	vector<string>	m_outputs;
	int				m_standBy_id;

	CNPC();
	virtual ~CNPC();
	
	string GetSpineName();

	virtual void Init();
	virtual void Update();
	virtual void OnCollisionEnter(CObject* collidedObject);
};
