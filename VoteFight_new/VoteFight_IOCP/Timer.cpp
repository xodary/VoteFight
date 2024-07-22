#include "pch.h"
#include "ImaysNet/Socket.h"
#include "ImaysNet/iocp.h"
#include "Timer.h"
#include "Bullet.h"
#include "GameScene.h"
#include "../Packet.h"
#include "Monster.h"
#include "RemoteClient.h"
#include "StateMachine.h"

//const int					numWorkerTHREAD{ 1 };	// Worker Thread Count
Iocp Iocp::iocp;
concurrency::concurrent_priority_queue<TIMER_EVENT> CTimer::timer_queue;
chrono::seconds phase_time[8] = { 150s, 90s,150s, 90s,120s, 60s,120s, 60s };


void CTimer::do_timer()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == CTimer::timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				CTimer::timer_queue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(10ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) {
			case EV_UPDATE:
			{
				// Monster Update
				for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::MONSTER]) {
					CMonster* monster = reinterpret_cast<CMonster*>(object.second);
					if (monster->m_dead) continue;
					monster->m_stateMachine->Update();
				}

				// Bullet Position Update
				vector<int>	deleteBullet;
				for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::BULLET]) {
					if (object.second == nullptr) continue;
					CBullet* bullet = reinterpret_cast<CBullet*>(object.second);
					auto duration = chrono::system_clock::now() - bullet->m_lastTime;
					auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
					XMFLOAT3 shift = Vector3::ScalarProduct(bullet->m_Vec, 0.1 * bullet->m_Velocity);
					bullet->m_Pos = Vector3::Add(bullet->m_Pos, shift);
					if (bullet->m_Pos.x > 400 || bullet->m_Pos.x < 0 ||
						bullet->m_Pos.z > 400 || bullet->m_Pos.z < 0)
					{
						deleteBullet.push_back(bullet->m_id);
						SC_DELETE_PACKET send_packet;
						send_packet.m_size = sizeof(send_packet);
						send_packet.m_type = P_SC_DELETE_PACKET;
						send_packet.m_itemID = bullet->m_id;
						send_packet.m_groupType = (int)GROUP_TYPE::BULLET;
						for (auto& rc : RemoteClient::m_remoteClients) {
							if (!rc.second->m_ingame) continue;
							rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
						}
						continue;
					}
					bullet->m_lastTime = chrono::system_clock::now();

					XMFLOAT4X4 matrix = Matrix4x4::Identity();
					//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Scale(client.second->m_player->m_Sca));
					//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Rotation(client.second->m_player->m_Rota));
					matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(object.second->m_Pos));
					object.second->m_origin.Transform(object.second->m_boundingBox, XMLoadFloat4x4(&matrix));

					for (auto& monsterobject : CGameScene::m_objects[(int)GROUP_TYPE::MONSTER]) {
						if (monsterobject.second->m_boundingBox.Intersects(object.second->m_boundingBox)) {
							cout << "Collide !" << endl;
							deleteBullet.push_back(bullet->m_id);
							CMonster* monster = reinterpret_cast<CMonster*>(monsterobject.second);
							if (monster->m_dead) continue;
							monster->m_Health -= 25 * 3;

							SC_ANIMATION_PACKET send_packet;
							send_packet.m_size = sizeof(send_packet);
							send_packet.m_type = P_SC_ANIMATION_PACKET;
							send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
							send_packet.m_id = monster->m_id;
							send_packet.m_loop = false;
							send_packet.m_bone = 0;	// Root

							if (monster->m_Health <= 0) {
								// Monster 사망
								monster->m_dead = true;
								strcpy_s(send_packet.m_key, "Dead");
							}
							else
							{
								strcpy_s(send_packet.m_key, "Gethit");
								monster->m_AnilastTime = chrono::system_clock::now();
								TIMER_EVENT ev{ (RemoteClient*)monster->m_id, chrono::system_clock::now() + CGameScene::m_animations["FishMon"]["Gethit"] , EV_ANIMATION, monster->m_id,(int)GROUP_TYPE::MONSTER,"idle",0 };
								CTimer::timer_queue.push(ev);
							}

							for (auto& rc : RemoteClient::m_remoteClients) {
								if (!rc.second->m_ingame) continue;
								rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
								cout << " >> send ) SC_ANIMATION_PACKET" << endl;
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
					}

					// Player와 닿았을 때
					for (auto& player : RemoteClient::m_remoteClients) {
						if (!player.second->m_ingame) continue;
						if (player.second->m_player->m_boundingBox.Intersects(object.second->m_boundingBox))
						{
							player.second->m_player->m_Health -= 25;
							cout << player.first << " : Health " << player.second->m_player->m_Health << endl;

							SC_HEALTH_CHANGE_PACKET send_packet;
							send_packet.m_size = sizeof(send_packet);
							send_packet.m_type = P_SC_HEALTH_CHANGE_PACKET;
							send_packet.m_id = player.second->m_id;
							send_packet.m_groupType = (int)GROUP_TYPE::PLAYER;
							send_packet.m_health = player.second->m_player->m_Health;
							for (auto& rc : RemoteClient::m_remoteClients) {
								if (!rc.second->m_ingame) continue;
								rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
								cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
							}
						}
					}

					SC_POS_PACKET send_packet;
					send_packet.m_size = sizeof(SC_POS_PACKET);
					send_packet.m_type = PACKET_TYPE::P_SC_POS_PACKET;
					send_packet.m_grouptype = (int)GROUP_TYPE::BULLET;
					send_packet.m_id = object.second->m_id;
					send_packet.m_pos = object.second->m_Pos;

					for (auto& rc : RemoteClient::m_remoteClients) {
						if (!rc.second->m_ingame) continue;
						rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					}
				}

				for (auto id : deleteBullet) {
					CGameScene::m_objects[(int)GROUP_TYPE::BULLET].erase(id);
				}

				for (auto& client : RemoteClient::m_remoteClients) {
					if (client.second == nullptr || !client.second->m_ingame) continue;
					auto duration = chrono::system_clock::now() - client.second->m_lastTime;
					auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
					XMFLOAT3 shift = Vector3::ScalarProduct(client.second->m_player->m_Vec, seconds * client.second->m_player->m_Velocity);
					client.second->m_player->m_Pos = Vector3::Add(client.second->m_player->m_Pos, shift);
					client.second->m_player->m_Pos.y = CGameScene::OnGetHeight(client.second->m_player->m_Pos.x, client.second->m_player->m_Pos.z);
					client.second->m_lastTime = chrono::system_clock::now();

					XMFLOAT4X4 matrix = Matrix4x4::Identity();
					//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Scale(client.second->m_player->m_Sca));
					//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Rotation(client.second->m_player->m_Rota));
					matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(client.second->m_player->m_Pos));
					client.second->m_player->m_origin.Transform(client.second->m_player->m_boundingBox, XMLoadFloat4x4(&matrix));

					for (int i = 0; i < (int)GROUP_TYPE::UI; ++i) {
						if (i == (int)GROUP_TYPE::PLAYER) continue;
						for (auto& object : CGameScene::m_objects[i]) {
							if (!client.second->m_player->m_collider) continue;
							if (!object.second->m_collider) continue;
							if (client.second->m_player->m_boundingBox.Intersects(object.second->m_boundingBox)) {
								client.second->m_player->m_Pos = Vector3::Subtract(client.second->m_player->m_Pos, shift);
								cout << "Collide !" << endl;
							}
						}
					}

					for (auto& rc : RemoteClient::m_remoteClients) {
						if (!rc.second->m_ingame) continue;
						SC_POS_PACKET send_packet;
						send_packet.m_size = sizeof(SC_POS_PACKET);
						send_packet.m_type = PACKET_TYPE::P_SC_POS_PACKET;
						send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
						send_packet.m_id = client.second->m_id;
						send_packet.m_pos = client.second->m_player->m_Pos;
						rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					}
				}
				ev.wakeup_time = chrono::system_clock::now() + 100ms;
				CTimer::timer_queue.push(ev);
			}
			break;
			case EV_PHASE:
			{
				for (auto& client : RemoteClient::m_remoteClients) {
					if (!client.second->m_ingame) continue;
					SC_UPDATE_PHASE_PACKET send_packet;
					send_packet.m_size = sizeof(SC_UPDATE_PHASE_PACKET);
					send_packet.m_type = PACKET_TYPE::P_SC_UPDATE_PHASE_PACKET;
					send_packet.m_phase = CTimer::phase;
					send_packet.m_time = phase_time[CTimer::phase];
					client.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));

					cout << "Phase " << CTimer::phase << endl;
				}
				ev.wakeup_time = chrono::system_clock::now() + phase_time[CTimer::phase++];
				CTimer::timer_queue.push(ev);
			}
			break;

			case EV_ANIMATION:
			{
				cout << "ANIMATION DONE" << endl;

				if (CGameScene::m_objects[ev.grouptype][ev.target_id]->m_dead) continue;
				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = ev.grouptype;
				send_packet.m_id = ev.target_id;
				send_packet.m_loop = true;
				send_packet.m_bone = ev.bone;

				strcpy_s(send_packet.m_key, CGameScene::m_objects[ev.grouptype][ev.target_id]->m_upAnimation.c_str());

				for (auto& rc : RemoteClient::m_remoteClients)
				{
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_ANIMATION_PACKET" << endl;
				}
			}
			break;

			}
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}