#include "pch.h"
#include "State.h"
#include "../Packet.h"
#include "Object.h"
#include "RemoteClient.h"
#include "Monster.h"
#include "StateMachine.h"
#include "GameScene.h"

CState::CState()
{
}

CState::~CState()
{
}

CMonsterIdleState::CMonsterIdleState()
{
}

CMonsterIdleState::~CMonsterIdleState()
{
}

void CMonsterIdleState::Enter(CObject* object)
{
	enterTime = chrono::system_clock::now();

	object->m_Velocity = 0;

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		strcpy_s(send_packet.m_key, "idle");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = object->m_Angle;
		cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
}

void CMonsterIdleState::Exit(CObject* object)
{
}

void CMonsterIdleState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	
	for (auto& rc : RemoteClient::m_remoteClients) {
		if (CGameScene::can_see(object->m_Pos, rc.second->m_player->m_Pos, 10)) {
			if (rc.second->m_player->m_dead) continue;
			monster->m_target = rc.second.get();
			monster->m_stateMachine->ChangeState(CMonsterChaseState::GetInstance());
			return;
		}
	}

	if (chrono::system_clock::now() - enterTime > 5s) {
		monster->m_stateMachine->ChangeState(CMonsterWalkState::GetInstance());
		return;
	}
}

CMonsterWalkState::CMonsterWalkState()
{
}

CMonsterWalkState::~CMonsterWalkState()
{
}

void CMonsterWalkState::Enter(CObject* object)
{
	enterTime = chrono::system_clock::now();

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		strcpy_s(send_packet.m_key, "Walk");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}

	object->m_Velocity = 5;
	float look = (rand() % 4) * 90;
	object->m_Angle = look;
	object->m_Vec = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, look, 0)));
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = look;
		cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
}

void CMonsterWalkState::Exit(CObject* object)
{
}

void CMonsterWalkState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	XMFLOAT3 shift = Vector3::ScalarProduct(object->m_Vec, 0.1 * object->m_Velocity);
	object->m_Pos = Vector3::Add(object->m_Pos, shift);
	if (object->m_Pos.x < 0 || object->m_Pos.x >= 400 ||
		object->m_Pos.z < 0 || object->m_Pos.z >= 400) {
		object->m_Pos = Vector3::Subtract(object->m_Pos, shift);
	}
	object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
	XMFLOAT4X4 matrix = Matrix4x4::Identity();
	matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object->m_Pos));
	object->m_origin.Transform(object->m_boundingBox, XMLoadFloat4x4(&matrix));

	for (auto& rc : RemoteClient::m_remoteClients) {
		if (rc.second->m_player->m_dead) continue;
		if (CGameScene::can_see(object->m_Pos, rc.second->m_player->m_Pos, 10)) {
			monster->m_target = rc.second.get();
			monster->m_stateMachine->ChangeState(CMonsterChaseState::GetInstance());
			return;
		}
	}
	if (chrono::system_clock::now() - enterTime > 5s) {
		monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
		return;
	}

	SC_POS_PACKET send_packet;
	send_packet.m_size = sizeof(SC_POS_PACKET);
	send_packet.m_type = PACKET_TYPE::P_SC_POS_PACKET;
	send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
	send_packet.m_id = object->m_id;
	send_packet.m_pos = object->m_Pos;
	for (auto& rc : RemoteClient::m_remoteClients) {
		if (!rc.second->m_ingame) continue;
		rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
	}
}

CMonsterChaseState::CMonsterChaseState()
{
}

CMonsterChaseState::~CMonsterChaseState()
{
}

void CMonsterChaseState::Enter(CObject* object)
{
	enterTime = chrono::system_clock::now();
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	object->m_Velocity = 7;
	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		strcpy_s(send_packet.m_key, "Run");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
}

void CMonsterChaseState::Exit(CObject* object)
{
}

void CMonsterChaseState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	XMFLOAT3 shift = Vector3::ScalarProduct(object->m_Vec, 0.1 * object->m_Velocity);
	object->m_Pos = Vector3::Add(object->m_Pos, shift);
	if (object->m_Pos.x < 0 || object->m_Pos.x >= 400 ||
		object->m_Pos.z < 0 || object->m_Pos.z >= 400) {
		object->m_Pos = Vector3::Subtract(object->m_Pos, shift);
	}
	object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
	XMFLOAT4X4 matrix = Matrix4x4::Identity();
	matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object->m_Pos));
	object->m_origin.Transform(object->m_boundingBox, XMLoadFloat4x4(&matrix));


	if (!CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 10)) {
		monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
		return;
	}

	if (CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 3)) {
		monster->m_stateMachine->ChangeState(CMonsterAttackState::GetInstance());
		return;
	}

	XMFLOAT3 from = XMFLOAT3(monster->m_target->m_player->m_Pos.x, 0, monster->m_target->m_player->m_Pos.z);
	XMFLOAT3 to = XMFLOAT3(monster->m_Pos.x, 0, monster->m_Pos.z);
	XMFLOAT3 fromto = Vector3::Normalize(Vector3::Subtract(from, to));
	float look = Vector3::Angle(XMFLOAT3(0, 0, 1), fromto);
	if (Vector3::CrossProduct(XMFLOAT3(0, 0, 1), fromto).y < 0)
		look = 360 - look;

	object->m_Angle = look;
	object->m_Vec = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, look, 0)));
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = look;
		cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}

	SC_POS_PACKET send_packet;
	send_packet.m_size = sizeof(SC_POS_PACKET);
	send_packet.m_type = PACKET_TYPE::P_SC_POS_PACKET;
	send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
	send_packet.m_id = object->m_id;
	send_packet.m_pos = object->m_Pos;
	for (auto& rc : RemoteClient::m_remoteClients) {
		if (!rc.second->m_ingame) continue;
		rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
	}
}

CMonsterAttackState::CMonsterAttackState()
{
}

CMonsterAttackState::~CMonsterAttackState()
{
}

void CMonsterAttackState::Enter(CObject* object)
{
	enterTime = chrono::system_clock::now();
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		strcpy_s(send_packet.m_key, "Attack");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
	
	object->m_Velocity = 0;
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = object->m_Angle;
		cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}

	monster->m_target->m_player->m_Health -= 10;
	{
		SC_HEALTH_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_HEALTH_CHANGE_PACKET;
		send_packet.m_id = monster->m_target->m_id;
		send_packet.m_groupType = (int)GROUP_TYPE::PLAYER;
		send_packet.m_health = monster->m_target->m_player->m_Health;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
}

void CMonsterAttackState::Exit(CObject* object)
{
}

void CMonsterAttackState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	if (chrono::system_clock::now() - enterTime > CGameScene::m_animations["FishMon"]["Attack"]) {
		if (CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 3)) {
			{
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				send_packet.m_id = object->m_id;
				send_packet.m_loop = true;
				send_packet.m_bone = 0;	// Root
				strcpy_s(send_packet.m_key, "Attack");
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
				}
			}
			monster->m_target->m_player->m_Health -= 10;
			{
				SC_HEALTH_CHANGE_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_HEALTH_CHANGE_PACKET;
				send_packet.m_id = monster->m_target->m_id;
				send_packet.m_groupType = (int)GROUP_TYPE::PLAYER;
				send_packet.m_health = monster->m_target->m_player->m_Health;
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
				}
			}
			enterTime = chrono::system_clock::now();
			if (monster->m_target->m_player->m_Health <= 0) {
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
				send_packet.m_id = monster->m_target->m_id;
				send_packet.m_loop = false;
				send_packet.m_bone = 0;	// Root
				strcpy_s(send_packet.m_key, "Death");
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
				}
				monster->m_target->m_player->m_dead = true;
				CGameScene::m_Rank[CGameScene::m_nowRank--] = monster->m_target->m_id;
				monster->m_stateMachine->ChangeState(CMonsterWalkState::GetInstance());
			}
		}
		else
			monster->m_stateMachine->ChangeState(CMonsterChaseState::GetInstance());
		return;
	}
}


CMonsterAttackedState::CMonsterAttackedState()
{
}

CMonsterAttackedState::~CMonsterAttackedState()
{
}

void CMonsterAttackedState::Enter(CObject* object)
{
	enterTime = chrono::system_clock::now();
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		strcpy_s(send_packet.m_key, "Gethit");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
	
	object->m_Velocity = 0;
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = object->m_Angle;
		cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}

	{
		SC_HEALTH_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_HEALTH_CHANGE_PACKET;
		send_packet.m_id = monster->m_id;
		send_packet.m_groupType = (int)GROUP_TYPE::MONSTER;
		send_packet.m_health = monster->m_Health;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
		}
	}
}

void CMonsterAttackedState::Exit(CObject* object)
{
}

void CMonsterAttackedState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	if (chrono::system_clock::now() - enterTime > CGameScene::m_animations["FishMon"]["Gethit"]) {
		monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
		return;
	}
}

