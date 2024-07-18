#include "pch.h"
#include "Box.h"
#include "UI.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameFramework.h"
#include "Player.h"
#include "InputManager.h"
#include "ImaysNet/PacketQueue.h"
#include "../Packet.h"

void CBox::Init()
{
	m_bilboardUI.push_back(new CSpeechBubbleUI(this));
}

void CBox::Update()
{
	CObject::Update();
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
					cout << "Item inside :" << endl;
					for (auto n : m_items)
						cout << n << endl;
					for (auto& b : m_bilboardUI)
						b->m_isActive = true;

					for (int i = 0; i < m_items.size(); ++i) {
						reinterpret_cast<CIcon*>(m_bilboardUI[1 + i])->centerX = (float)-BUBBLE_WIDTH + ((float)BUBBLE_WIDTH*2 / m_items.size()) * i + ((float)BUBBLE_WIDTH*2 / m_items.size()) / 2;
						reinterpret_cast<CIcon*>(m_bilboardUI[1 + i])->centerY = 0;
					}

				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "Item ¸Ö¾îÁü" << endl;
				for (auto& b : m_bilboardUI)
					b->m_isActive = false;
			}

			if (m_bilboardUI[0]->m_isActive && KEY_TAP(KEY::SPACE)) {
				CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
				CPlayer* player = reinterpret_cast<CPlayer*>(scene->GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id));
				for (const string& item : m_items) {
					player->GetItem(item);
				}
				CS_TAKEOUT_PACKET p;
				p.m_type = P_CS_TAKEOUT_PACKET;
				p.m_size = sizeof(p);
				p.m_groupType = (int)GROUP_TYPE::BOX;
				p.m_itemID = m_id;
				PacketQueue::AddSendPacket(&p);
			}
		}
	}
}

void COnceItem::Init()
{
	m_bilboardUI.push_back(new CSpeechBubbleUI(this));
}

void COnceItem::Update()
{
	CObject::Update();
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
					cout << "Item inside :" << endl;
					for (auto n : m_items)
						cout << n << endl;
					for (auto& b : m_bilboardUI)
						b->m_isActive = true;
				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "Item ¸Ö¾îÁü" << endl;
				for (auto& b : m_bilboardUI)
					b->m_isActive = false;
			}

			if (m_bilboardUI[0]->m_isActive && KEY_TAP(KEY::SPACE)) {
				CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
				CPlayer* player = reinterpret_cast<CPlayer*>(scene->GetIDObject(GROUP_TYPE::PLAYER, CGameFramework::GetInstance()->my_id));
				for (const string& item : m_items) {
					player->GetItem(item);
				}
				CS_TAKEOUT_PACKET p;
				p.m_type = P_CS_TAKEOUT_PACKET;
				p.m_size = sizeof(p);
				p.m_groupType = (int)GROUP_TYPE::ONCE_ITEM;
				p.m_itemID = m_id;
				PacketQueue::AddSendPacket(&p);
			}
		}
	}
}
