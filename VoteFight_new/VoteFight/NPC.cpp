#include "pch.h"
#include "NPC.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "StateMachine.h"
#include "Transform.h"
#include "NPCStates.h"
#include "UI.h"

CNPC::CNPC() :
	m_spineName("mixamorig:Spine")
{
	SetName("NPC");
	SetGroupType((UINT)GROUP_TYPE::NPC);
}

CNPC::~CNPC()
{
}


string CNPC::GetSpineName()
{
	return m_spineName;
}

void CNPC::Init()
{	
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	stateMachine->SetCurrentState(CNPCIdleState::GetInstance());

	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->SetWeight("idle", 1.0f);
	animator->Play("idle", true);

	m_bilboardUI.push_back(new CSpeechBubbleUI(this));
}

void CNPC::Update()
{
	CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
	if (scene->m_name == "GameScene")
	{
		CObject* object = scene->GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id);
		XMFLOAT3 player_pos = object->GetPosition();
		XMFLOAT3 my_pos = GetPosition();

		if (!m_bilboardUI.empty())
		{
			if (abs(player_pos.x - my_pos.x) <= NPC_RANGE && abs(player_pos.z - my_pos.z) <= NPC_RANGE)
			{
				if (!m_bilboardUI[0]->m_isActive) {
					cout << "NPC needs" << endl;
					cout << "need: " << endl;
					for (auto n : m_needs)
						cout << n << endl;
					cout << "output: " << endl;
					for (auto o : m_outputs)
						cout << o << endl;
					m_bilboardUI[0]->m_isActive = true;
				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "NPC ¸Ö¾îÁü" << endl;
				m_bilboardUI[0]->m_isActive = false;
			}
		}
	}
	CCharacter::Update();
}

void CNPC::OnCollisionEnter(CObject* collidedObject)
{
	if (collidedObject->GetGroupType() == (UINT)GROUP_TYPE::PLAYER)
	{
		// if (!m_Quest->getCompletionStatus())
		// {
		// 	cout << "¾È³ç!";
		// 	m_Quest->show();
		// }
	}
}

