#pragma once
#include "Character.h"

class CQuest;
class C_NPC_Item;
class CBilboardUI;

class CNPC : public CCharacter
{
private:
	string	 m_spineName;

   CBilboardUI* speech_bubble;
public:
	vector<string> m_needs;
	vector<string> m_outputs;

	CNPC();
	virtual ~CNPC();
	
	string GetSpineName();

	virtual void Init();
	virtual void Update();
	virtual void OnCollisionEnter(CObject* collidedObject);
};
