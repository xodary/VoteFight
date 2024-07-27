#include "pch.h"
#include "NPC.h"
#include "GameFramework.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Animator.h"
#include "InputManager.h"
#include "SoundManager.h"
#include "StateMachine.h"
#include "Transform.h"
#include "Player.h"
#include "UI.h"
#include "ImaysNet/PacketQueue.h"
#include "../Packet.h"

CNPC::CNPC() :
	m_spineName("mixamorig:Spine")
{
	SetName("NPC");
	SetGroupType((UINT)GROUP_TYPE::NPC);
}

CNPC::~CNPC()
{
}

void CNPC::Init()
{	
	CAnimator* animator = static_cast<CAnimator*>(GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->SetAnimateBone(FindFrame("mixamorig:Hips"), ANIMATION_BONE::ROOT);
	animator->SetWeight("idle", ANIMATION_BONE::ROOT, 1.0f);
	animator->Play("idle", true);

	m_bilboardUI.push_back(new CSpeechBubbleUI(this));

	CIcon* ui = new CIcon(this, "arrow");
	m_bilboardUI.push_back(ui);
	// m_bilboardUI.push_back(new CIcon(this));
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
					for(auto& b : m_bilboardUI)
						b->m_isActive = true;

					for (int i = 0; i < m_needs.size(); ++i) {
						reinterpret_cast<CIcon*>(m_bilboardUI[2 + i])->centerX = (float)-BUBBLE_WIDTH + ((float)BUBBLE_WIDTH*2 / m_needs.size()) * i + ((float)BUBBLE_WIDTH*2 / m_needs.size()) / 2;
						reinterpret_cast<CIcon*>(m_bilboardUI[2 + i])->centerY = 0.8f;
					}
					for (int i = 0; i < m_outputs.size(); ++i) {
						reinterpret_cast<CIcon*>(m_bilboardUI[2 + m_needs.size() + i])->centerX = (float)-BUBBLE_WIDTH + ((float)BUBBLE_WIDTH*2 / m_outputs.size()) * i + ((float)BUBBLE_WIDTH*2 / m_outputs.size()) / 2;
						reinterpret_cast<CIcon*>(m_bilboardUI[2 + m_needs.size() + i])->centerY = -0.7f;
					}

				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "NPC ¸Ö¾îÁü" << endl;
				for (auto& b : m_bilboardUI)
					b->m_isActive = false;
			}

			if (m_bilboardUI[0]->m_isActive && KEY_TAP(KEY::SPACE)) {
				CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
				CPlayer* player = reinterpret_cast<CPlayer*>(scene->GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id));
				bool allFound = true;

				auto new_item = player->myItems;
				for (const string& need : m_needs) {
					auto it = find_if(new_item.rbegin(), new_item.rend(), [&need](CItem a) {
						return a.m_name == need;
						});
					if (it == new_item.rend()) allFound = false;
					else {
						it->m_capacity -= 1;
						if (it->m_capacity <= 0) {
							it->m_name.clear();
						}
					}
				}

				if(allFound) {
					for (const string& need : m_needs) {
						player->myItems = new_item;
					}

					for (const string& output : m_outputs) {
						if(output == "bullets") player->GetItem(output, 10);
						else player->GetItem(output, 1);
					}
					CSoundManager::GetInstance()->Play(SOUND_TYPE::EXCHANGE, 0.3f, true);
					CS_EXCHANGE_DONE_PACKET p;
					p.m_type = P_CS_EXCHANGE_DONE_PACKET;
					p.m_size = sizeof(p);
					p.m_npc_id = m_id;
					PacketQueue::AddSendPacket(&p);
				}
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

