#include "pch.h"
#include "Box.h"
#include "UI.h"
#include "SceneManager.h"
#include "Scene.h"
#include "GameFramework.h"

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
					m_bilboardUI[0]->m_isActive = true;
				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "Item ¸Ö¾îÁü" << endl;
				m_bilboardUI[0]->m_isActive = false;
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
					m_bilboardUI[0]->m_isActive = true;
				}
			}
			else if (m_bilboardUI[0]->m_isActive) {
				cout << "Item ¸Ö¾îÁü" << endl;
				m_bilboardUI[0]->m_isActive = false;
			}
		}
	}
}
