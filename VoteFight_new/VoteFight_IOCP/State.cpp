#include "pch.h"
#include "State.h"
#include "../Packet.h"
#include "Object.h"
#include "Timer.h"
#include "RemoteClient.h"
#include "Monster.h"
#include "StateMachine.h"
#include "GameScene.h"

unsigned int CGameScene::m_nowRank;
unsigned int CGameScene::m_Rank[3];
int CTimer::phase;

XMFLOAT2 GoUp[4] = { {398, 7.5}, {278, 114.5}, {193.7, 207.3}, {0, 0} };

CState::CState()
{
}

CState::~CState()
{
}


CMonsterGoUpState::CMonsterGoUpState()
{
}

CMonsterGoUpState::~CMonsterGoUpState()
{
}

void CMonsterGoUpState::Enter(CObject* object)
{
}

void CMonsterGoUpState::Exit(CObject* object)
{
}

void CMonsterGoUpState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	
	if (CTimer::phase % 2 == 0) {
		if ((GoUp[CTimer::phase / 2 - 1].x < object->m_Pos.x || 
			GoUp[CTimer::phase / 2 - 1].y > object->m_Pos.z ) &&
			object->m_Velocity != 5) {
			if(GoUp[CTimer::phase / 2 - 1].x < object->m_Pos.x) object->m_Angle = 270;
			else object->m_Angle = 0;
			object->m_Velocity = 7;
			cout << "Go Up" << endl;
			object->m_Vec = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, object->m_Angle, 0)));
			{
				SC_VELOCITY_CHANGE_PACKET send_packet;
				send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
				send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
				send_packet.m_id = object->m_id;
				send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				send_packet.m_vel = object->m_Velocity;
				send_packet.m_pos = object->m_Pos;
				send_packet.m_look = object->m_Angle;
				send_packet.m_angle = object->m_Angle;

				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				}
			}
			
			{
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				send_packet.m_id = object->m_id;
				send_packet.m_loop = true;
				send_packet.m_bone = 0;	// Root
				send_packet.m_sound = -1;
				strcpy_s(send_packet.m_key, "Walk");
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				}
			}
		}
		if(GoUp[CTimer::phase / 2 - 1].x >= object->m_Pos.x && GoUp[CTimer::phase / 2 - 1].y <= object->m_Pos.z) {
			monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
			return;
		}

		// 물위로 이동
		XMFLOAT3 shift = Vector3::ScalarProduct(object->m_Vec, 0.1 * object->m_Velocity);
		object->m_Pos = Vector3::Add(object->m_Pos, shift);
		object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
		
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
}

CMonsterIdleState::CMonsterIdleState()
{
}

CMonsterIdleState::~CMonsterIdleState()
{
}

void CMonsterIdleState::Enter(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	monster->stateTime = chrono::system_clock::now() + std::chrono::seconds(rand() % 5 + 1);

	object->m_Velocity = 0;

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		send_packet.m_sound = -1;
		strcpy_s(send_packet.m_key, "idle");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
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
		send_packet.m_angle = object->m_Angle;

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
	
	if (CTimer::phase % 2 == 1) {
		monster->m_stateMachine->ChangeState(CMonsterGoUpState::GetInstance());
	}
	for (auto& rc : RemoteClient::m_remoteClients) {
		if (CGameScene::can_see(object->m_Pos, rc.second->m_player->m_Pos, 15)) {
			if (rc.second->m_player->m_dead) continue;
			monster->m_target = rc.second.get();
			monster->m_stateMachine->ChangeState(CMonsterChaseState::GetInstance());
			return;
		}
	}

	if (chrono::system_clock::now() > monster->stateTime) {
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
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	monster->stateTime = chrono::system_clock::now() + std::chrono::seconds(rand() % 5 + 1);

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		send_packet.m_sound = -1;
		strcpy_s(send_packet.m_key, "Walk");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
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
		send_packet.m_angle = look;

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
	XMFLOAT3 origin_pos = object->m_Pos;
	object->m_Pos = Vector3::Add(object->m_Pos, shift);
	if (object->m_Pos.x < 0 || object->m_Pos.x >= 400 ||
		object->m_Pos.z < 0 || object->m_Pos.z >= 400) {
		object->m_Pos = Vector3::Subtract(object->m_Pos, shift);
	}
	object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
	XMFLOAT4X4 matrix = Matrix4x4::Identity();
	matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object->m_Pos));
	object->m_origin.Transform(object->m_boundingBox, XMLoadFloat4x4(&matrix));

	for (int i = 0; i < (int)GROUP_TYPE::UI; ++i) {
		if (i == (int)GROUP_TYPE::PLAYER) continue;
		if (i == (int)GROUP_TYPE::ONCE_ITEM) continue;
		for (auto& structure : CGameScene::m_objects[i]) {
			if (!structure.second->m_collider) continue;
			if (!object->m_collider) continue;
			if (structure.second->m_boundingBox.Intersects(object->m_boundingBox)) {
				structure.second->m_Pos = origin_pos;
			}
		}
	}

	for (auto& rc : RemoteClient::m_remoteClients) {
		if (rc.second->m_player->m_dead) continue;
		if (CGameScene::can_see(object->m_Pos, rc.second->m_player->m_Pos, 15)) {
			monster->m_target = rc.second.get();
			monster->m_stateMachine->ChangeState(CMonsterChaseState::GetInstance());
			return;
		}
	}
	if (chrono::system_clock::now() > monster->stateTime) {
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
		send_packet.m_sound = -1;
		strcpy_s(send_packet.m_key, "Run");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
}

void CMonsterChaseState::Exit(CObject* object)
{
}

void CMonsterChaseState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	
	XMFLOAT3 from = XMFLOAT3(monster->m_target->m_player->m_Pos.x, 0, monster->m_target->m_player->m_Pos.z);
	XMFLOAT3 to = XMFLOAT3(monster->m_Pos.x, 0, monster->m_Pos.z);
	XMFLOAT3 fromto = Vector3::Normalize(Vector3::Subtract(from, to));
	float look = Vector3::Angle(XMFLOAT3(0, 0, 1), fromto);
	if (Vector3::CrossProduct(XMFLOAT3(0, 0, 1), fromto).y < 0)
		look = 360 - look;

	object->m_Angle = look;
	object->m_Vec = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, look, 0)));

	XMFLOAT3 shift = Vector3::ScalarProduct(object->m_Vec, 0.1 * object->m_Velocity);
	XMFLOAT3 origin_pos = object->m_Pos;
	object->m_Pos = Vector3::Add(object->m_Pos, shift);
	if (object->m_Pos.x < 0 || object->m_Pos.x >= 400 ||
		object->m_Pos.z < 0 || object->m_Pos.z >= 400) {
		object->m_Pos = Vector3::Subtract(object->m_Pos, shift);
	}
	object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
	XMFLOAT4X4 matrix = Matrix4x4::Identity();
	matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object->m_Pos));
	object->m_origin.Transform(object->m_boundingBox, XMLoadFloat4x4(&matrix));

	for (int i = 0; i < (int)GROUP_TYPE::UI; ++i) {
		if (i == (int)GROUP_TYPE::PLAYER) continue;
		if (i == (int)GROUP_TYPE::ONCE_ITEM) continue;
		for (auto& structure : CGameScene::m_objects[i]) {
			if (!structure.second->m_collider) continue;
			if (!object->m_collider) continue;
			if (structure.second->m_boundingBox.Intersects(object->m_boundingBox)) {
				structure.second->m_Pos = origin_pos;
			}
		}
	}

	if (!CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 15)) {
		monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
		return;
	}

	if (CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 2)) {
		monster->m_stateMachine->ChangeState(CMonsterAttackState::GetInstance());
		return;
	}
	{
		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = object->m_id;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_vel = object->m_Velocity;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_look = look;
		send_packet.m_angle = look;

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
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	monster->enterTime = chrono::system_clock::now();

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		send_packet.m_sound = -1;
		strcpy_s(send_packet.m_key, "Attack");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
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
		send_packet.m_angle = object->m_Angle;

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
		send_packet.m_damage = 10;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
}

void CMonsterAttackState::Exit(CObject* object)
{
}

void CMonsterAttackState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	if (chrono::system_clock::now() - monster->enterTime > CGameScene::m_animations["FishMon"]["Attack"]) {
		if (CGameScene::can_see(object->m_Pos, monster->m_target->m_player->m_Pos, 2)) {
			{
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				send_packet.m_id = object->m_id;
				send_packet.m_loop = true;
				send_packet.m_bone = 0;	// Root
				send_packet.m_sound = -1;
				strcpy_s(send_packet.m_key, "Attack");
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
				send_packet.m_damage = 10;
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				}
			}
			monster->enterTime = chrono::system_clock::now();
			if (monster->m_target->m_player->m_Health <= 0) {
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
				send_packet.m_id = monster->m_target->m_id;
				send_packet.m_loop = false;
				send_packet.m_bone = 0;	// Root
				send_packet.m_sound = -1;
				strcpy_s(send_packet.m_key, "Death");
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
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
	CMonster* monster = reinterpret_cast<CMonster*>(object);
	monster->enterTime = chrono::system_clock::now();

	{
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
		send_packet.m_id = object->m_id;
		send_packet.m_loop = true;
		send_packet.m_bone = 0;	// Root
		send_packet.m_sound = -1;
		strcpy_s(send_packet.m_key, "Gethit");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
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
		send_packet.m_angle = object->m_Angle;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
}

void CMonsterAttackedState::Exit(CObject* object)
{
}

void CMonsterAttackedState::Update(CObject* object)
{
	CMonster* monster = reinterpret_cast<CMonster*>(object);

	if (chrono::system_clock::now() - monster->enterTime > CGameScene::m_animations["FishMon"]["Gethit"]) {
		monster->m_stateMachine->ChangeState(CMonsterIdleState::GetInstance());
		return;
	}
}

