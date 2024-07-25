#pragma once
#include "pch.h"
#include "./ImaysNet/ImaysNet.h"
#include "RemoteClient.h"
#include "Timer.h"
#include "GameScene.h"
#include "Item.h"
#include "NPC.h"
#include "Bullet.h"
#include "Box.h"
#include "State.h"
#include "Monster.h"
#include "StateMachine.h"

volatile bool				stopServer = false;
volatile bool				GameStart = false;
static unsigned long long	nextClientID{ 0 };		// Next Client ID

vector<shared_ptr<thread>>	workerThreads;			// Worker Thread Vector
enum class IO_TYPE;									// I/O Type

// 클라이언트들을 가리키는 포인터를 저장하는 벡터
vector<RemoteClient*>		remoteClients_ptr_v;

// 삭제 클라이언트 목록
list<shared_ptr<RemoteClient>>		deleteClinets;

recursive_mutex				mx_accept;				
shared_ptr<Socket>			listenSocket;		
shared_ptr<RemoteClient>	remoteClientCandidate; 
unordered_map<int, CObject*> CGameScene::m_objects[(int)GROUP_TYPE::COUNT];
unordered_map<string, unordered_map<string, std::chrono::system_clock::duration>> CGameScene::m_animations;
int	CTimer::phase;
int bullet_id = 0;
int item_id = 0;

// Client 종료 처리 함수
void	ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient);
void	ProcessAccept();
void	PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet);
void	WorkerThread();
void	CloseServer();

// 서버 프로그램 진입점
int main(int argc, char* argv[])
{
	CGameScene::LoadTerrain("HeightMap.bin");
	CGameScene::Load("ServerScene.bin");
	CGameScene::Load("GameScene.bin");
	CGameScene::NPCInitialize();
	CGameScene::LoadSkinningAnimations();

	// Create listen socket & Binding
	listenSocket = make_shared<Socket>(SocketType::Tcp);
	listenSocket->Bind(Endpoint("0.0.0.0", SERVER_PORT));
	listenSocket->Listen();

	// IOCP Init & Add
	int num_threads = std::thread::hardware_concurrency() - 1;
	Iocp::iocp.Add(*listenSocket,listenSocket.get());

	// Client Connect Ready
	remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
	string errorText;

	// Asynchronously waits for Client Connections
	if (!listenSocket->AcceptOverlapped(remoteClientCandidate->m_tcpConnection, errorText)
		&& WSAGetLastError() != ERROR_IO_PENDING) {
		throw Exception(" [ Exception ] : Overlapped AcceptEx failed.");
	}
	listenSocket->m_isReadOverlapped = true;

	// Worket Thread
	for (int i = 0 ; i < num_threads; ++i)
		workerThreads.emplace_back(make_shared<thread>(WorkerThread));

	thread timer_thread{ CTimer::do_timer };
	timer_thread.join();

	for (auto& th : workerThreads) 
		th->join();

	// Sever End
	CloseServer();
}

void ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient)
{
	// 에러 혹은 소켓 종료 시
	_remoteClient->m_tcpConnection.Close();
	{
		RemoteClient::m_remoteClients.erase(_remoteClient.get());	// 클라이언트 제거

		if (_remoteClient->m_ingame) {
			SC_DELETE_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_DELETE_PACKET;
			send_packet.m_groupType = (int)GROUP_TYPE::PLAYER;
			send_packet.m_itemID = _remoteClient->m_id;

			for (auto& rc : RemoteClient::m_remoteClients) {
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}
		cout << " >> Client left. There are " << RemoteClient::m_remoteClients.size() << " connections.\n";
	}
}
void WorkerThread()
{
	try {
		while (!stopServer) {
			// I/O 완료 이벤트 대기
			IocpEvents readEvents;
			Iocp::iocp.Wait(readEvents, 100);

			// 받은 이벤트 처리
			for (int i = 0; i < readEvents.m_eventCount; ++i) {
				auto& readEvent = readEvents.m_events[i];
				auto p_readOverlapped = (EXP_OVER*)readEvent.lpOverlapped;

				if (IO_TYPE::IO_SEND == p_readOverlapped->m_ioType) {
					// cout << " >> Send - size : " << (int)p_readOverlapped->m_buf[0] << endl;
					// cout << " >> Send - type : " << (int)p_readOverlapped->m_buf[1] << endl;
					p_readOverlapped->m_isReadOverlapped = false;
					continue;

				}

				if (readEvent.lpCompletionKey == (ULONG_PTR)listenSocket.get()) {    // Listetn socket
					ProcessAccept(); // Client Accept
				}
				else { // TCP Socket
					shared_ptr<RemoteClient> remoteClient;	// 처리할 클라이언트 가져오기
					{
						lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
						remoteClient = RemoteClient::m_remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					}

					if (remoteClient) { // 수신 완료 상태. 완료된 것 꺼내서 작업
						remoteClient->m_tcpConnection.m_isReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0) { //  0 - TCP 연결 종료, -1 : Error
							ProcessClientLeave(remoteClient);
						}
						else {
							// 수신 완료 상태. 완료된 것 꺼내서 작업
							char* recv_buf = remoteClient->m_tcpConnection.m_recvOverlapped.m_buf;
							int recv_buf_Length = ec;

							// cout << " >> Recv - recv_buf_Length : " << recv_buf_Length << endl;

							{	// 패킷 처리
								int remain_data = recv_buf_Length + remoteClient->m_tcpConnection.m_prev_remain;
								while (remain_data > 0) {
									unsigned char packet_size = recv_buf[0];
									if (packet_size > remain_data) // // 남은 데이터가 현재 처리할 패킷 크기보다 적으면 잘렸거나 딱 떨어짐.
										break;

									//패킷 처리
									PacketProcess(remoteClient, recv_buf);

									//다음 패킷 이동, 남은 데이터 갱신
									recv_buf += packet_size;
									remain_data -= packet_size;

								}
								//남은 데이터 저장
								remoteClient->m_tcpConnection.m_prev_remain = remain_data;

								//남은 데이터가 0이 아닌 값을 가지면 recv_buf의 맨 앞으로 복사
								if (remain_data > 0) {
									memcpy(remoteClient->m_tcpConnection.m_recvOverlapped.m_buf, recv_buf, remain_data);
								}
							}

							// 수신 받을 준비
							if (remoteClient->m_tcpConnection.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING) {
								ProcessClientLeave(remoteClient);
							}
							else {
								// I/O를 걸고 완료를 대기 상태로 변경
								remoteClient->m_tcpConnection.m_isReadOverlapped = true;
							}
						}
					}
				}
			}
		}
	}
	catch (Exception& e) {
		cout << " [ Exception ] " << e.what() << endl;
	}
}

void ProcessAccept()
{
	lock_guard<recursive_mutex> lock_accept(mx_accept);
	listenSocket->m_isReadOverlapped = false;
	// Accept은 이미 완료 -> 마무리 작업
	if (remoteClientCandidate->m_tcpConnection.UpdateAcceptContext(*listenSocket) != 0) {
		listenSocket->Close();
	}
	else {
		shared_ptr<RemoteClient> remoteClient = remoteClientCandidate;
		remoteClients_ptr_v.emplace_back(remoteClient.get());

		// 소켓 IOCP에 추가
		Iocp::iocp.Add(remoteClient->m_tcpConnection, remoteClient.get());

		// overlapped 수신 요청
		if (remoteClient->m_tcpConnection.ReceiveOverlapped() != 0
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			remoteClient->m_tcpConnection.Close();
		}
		else {
			// I/O를 걸고 완료를 대기 상태로 변경
			remoteClient->m_tcpConnection.m_isReadOverlapped = true; 
			{
				lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
				RemoteClient::m_remoteClients.insert({ remoteClient.get(), remoteClient });

				cout << " >> Client joined. There are " << RemoteClient::m_remoteClients.size() << " connections.\n";
			}
		}

		// 계속해서 소켓 받아야 하므로 Listen 소켓애 overlapped I/O를 걸어둠
		remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
		string errorText;
		if (!listenSocket->AcceptOverlapped(remoteClientCandidate->m_tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			listenSocket->Close();
		}
		else {
			// Listen 소켓은 연결을 기다리는 상태
			listenSocket->m_isReadOverlapped = true;
		}
	}
}
void PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet)
{
	switch (_Packet[1]) {

	case PACKET_TYPE::P_CS_LOGIN_PACKET:
	{
		if (GameStart) {
			SC_LOGIN_FAIL_PACKET send_packet;
			send_packet.m_size = sizeof(SC_LOGIN_FAIL_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_LOGIN_FAIL_PACKET;
			send_packet.m_fail_type = 0;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			ProcessClientLeave(_Client);
			return;
		}

		// 자신을 포함하여 4명
		if (RemoteClient::m_remoteClients.size() == 4) {
			SC_LOGIN_FAIL_PACKET send_packet;
			send_packet.m_size = sizeof(SC_LOGIN_FAIL_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_LOGIN_FAIL_PACKET;
			send_packet.m_fail_type = 1;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			ProcessClientLeave(_Client);
			return;
		}

		CS_LOGIN_PACKET* recv_packet = reinterpret_cast<CS_LOGIN_PACKET*>(_Packet);
		_Client->m_id = nextClientID++;
		_Client->m_name = string(recv_packet->m_name);
		_Client->m_player = make_shared<CPlayer>(XMFLOAT3());
		_Client->m_player->m_collider = true;
		_Client->m_player->m_origin.Center = XMFLOAT3(0, 1.59f, 0);
		_Client->m_player->m_origin.Extents = XMFLOAT3(1, 2.14f, 1);
		_Client->m_ingame = true;
		_Client->m_player->m_grouptype = (int)GROUP_TYPE::PLAYER;

		SC_LOGIN_OK_PACKET send_packet;
		send_packet.m_size = sizeof(SC_LOGIN_OK_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_LOGIN_OK_PACKET;
		send_packet.m_id = _Client->m_id;
		_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		cout << " >> Send ) LOGIN_OK_PACKET - Client ID : " << _Client->m_id << ", Client Name : " << _Client->m_name << endl;
		cout << " >> Size of RemoteClient: " << RemoteClient::m_remoteClients.size() << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			if (rc.second->m_char < 0) continue;
			SC_SELECT_PACKET send_packet;
			send_packet.m_size = sizeof(SC_SELECT_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_SELECT_PACKET;
			send_packet.m_id = rc.second->m_id;
			send_packet.m_char = rc.second->m_char;
			strcpy_s(send_packet.m_name, rc.second->m_name.c_str());

			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}

	}
	break;

	case PACKET_TYPE::P_CS_STATE_ENTER_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		CS_STATE_ENTER_PACKET* recv_packet = reinterpret_cast<CS_STATE_ENTER_PACKET*>(_Packet);
		cout << "P_CS_STATE_ENTER_PACKET" << endl;

		auto duration = chrono::system_clock::now() - _Client->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		XMFLOAT3 shift = Vector3::ScalarProduct(_Client->m_player->m_Vec, seconds * _Client->m_player->m_Velocity);
		_Client->m_player->m_Pos = Vector3::Add(_Client->m_player->m_Pos, shift);
		_Client->m_lastTime = chrono::system_clock::now();

		if (recv_packet->m_state == 0 || recv_packet->m_state == 2)
		{
			_Client->m_player->m_Velocity = 0;

			SC_VELOCITY_CHANGE_PACKET send_packet;
			send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
			send_packet.m_id = _Client->m_id;
			send_packet.m_grouptype = _Client->m_player->m_grouptype;
			send_packet.m_vel = _Client->m_player->m_Velocity;
			send_packet.m_pos = _Client->m_player->m_Pos;
			send_packet.m_look = -1;
			cout << " >> send ) CS_VELOCITY_CHANGE_PACKET" << endl;

			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}

		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(SC_ANIMATION_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = _Client->m_player->m_grouptype;
		send_packet.m_id = _Client->m_id;
		send_packet.m_loop = true;
		if(_Client->m_player->upperAnimationFinished)
			send_packet.m_bone = 0;	// Root
		else
			send_packet.m_bone = 2;	// Lower

		switch (_Client->m_player->m_Weapon)
		{
		case 0:	// Punch
			switch (recv_packet->m_state)
			{
			case 0:		// idle
			case 2:		// focusidle
				strcpy_s(send_packet.m_key, "Idle");
				break;
			case 1:		// walk
				strcpy_s(send_packet.m_key, "Run");
				break;
			case 3:		// focuswalk
				strcpy_s(send_packet.m_key, "Walk");
				break;
			}
			break;
		case 1:	// Pistol
			switch (recv_packet->m_state)
			{
			case 0:		// idle
				strcpy_s(send_packet.m_key, "Pistol_idle");
				break;
			case 1:		// walk
				strcpy_s(send_packet.m_key, "Pistol_run");
				break;
			case 2:		// focusidle
				strcpy_s(send_packet.m_key, "Pistol_focus");
				break;
			case 3:		// focuswalk
				strcpy_s(send_packet.m_key, "Pistol_slowwalk");
				break;
			}
			break;
		case 2:
			switch (recv_packet->m_state)
			{
			case 0:		// idle
			case 2:		// focusidle
				strcpy_s(send_packet.m_key, "Pistol_idle");
				break;
			case 1:		// walk
				strcpy_s(send_packet.m_key, "Pistol_run");
				break;
			case 3:		// focuswalk
				strcpy_s(send_packet.m_key, "Weapon_slowwalk");
				break;
			}
			break;

		}
		_Client->m_player->m_upAnimation = send_packet.m_key;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		cout << " >> send ) CS_WALK_ENTER_PACKET" << endl;
	}
	break;

	case PACKET_TYPE::P_CS_VELOCITY_CHANGE_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		CS_VELOCITY_CHANGE_PACKET* recv_packet = reinterpret_cast<CS_VELOCITY_CHANGE_PACKET*>(_Packet);

		auto duration = chrono::system_clock::now() - _Client->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		XMFLOAT3 shift = Vector3::ScalarProduct(_Client->m_player->m_Vec, seconds * _Client->m_player->m_Velocity);
		_Client->m_player->m_Pos = Vector3::Add(_Client->m_player->m_Pos, shift);

		if (recv_packet->m_Rbutton) _Client->m_player->m_Velocity = 5;
		else {
			if (recv_packet->m_shift) _Client->m_player->m_Velocity = 15;
			else _Client->m_player->m_Velocity = 10;
		}

		XMFLOAT3 vector = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, recv_packet->m_angle, 0)));
		cout << seconds << endl;
		_Client->m_lastTime = chrono::system_clock::now();
		_Client->m_player->m_Vec = vector;
		_Client->m_player->m_Angle = recv_packet->m_angle;

		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = _Client->m_id;
		send_packet.m_angle = recv_packet->m_angle;
		if (recv_packet->m_Rbutton)
			send_packet.m_look = -1;
		else
			send_packet.m_look = recv_packet->m_angle;
		send_packet.m_grouptype = _Client->m_player->m_grouptype;
		send_packet.m_vel = _Client->m_player->m_Velocity;
		send_packet.m_pos = _Client->m_player->m_Pos;
		cout << " >> send ) CS_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		break;
	}
	break;

	case PACKET_TYPE::P_CS_SELECT_PACKET:
	{
		CS_SELECT_PACKET* recv_packet = reinterpret_cast<CS_SELECT_PACKET*>(_Packet);
		_Client->m_ready = true;
		_Client->m_char = recv_packet->m_char;

		SC_SELECT_PACKET send_packet;
		send_packet.m_size = sizeof(SC_SELECT_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_SELECT_PACKET;
		send_packet.m_id = _Client->m_id;
		send_packet.m_char = _Client->m_char;
		strcpy_s(send_packet.m_name, _Client->m_name.c_str());
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}

		if (RemoteClient::m_remoteClients.size() < MAX_PLAYER) return;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			if (!rc.second->m_ready) {
				return;
			}
		}

		cout << " GameStart" << endl;
		{
			CGameScene::m_nowRank = 2;
			//GameStart = true;
			SC_GAMESTART_PACKET send_packet;
			send_packet.m_size = sizeof(SC_GAMESTART_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_GAMESTART_PACKET;

			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}
		cout << " >> send ) SC_GAMESTART_PACKET" << endl;

		//XMFLOAT3 pos[3]{ {10, 0, 10},{380, 0, 16},{388, 0, 382} };
		XMFLOAT3 pos[3]{ {10, 0, 10},{20, 0, 10},{30, 0, 10} };
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			CGameScene::m_objects[(int)GROUP_TYPE::PLAYER][rc.second->m_id] = rc.second->m_player.get();
			int i = 0;
			for (auto& rc2 : RemoteClient::m_remoteClients) {	// other players
				if (!rc2.second->m_ingame) continue;
				rc2.second->m_player->m_Pos = pos[i++];

				SC_SPAWN_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_SPAWN_PACKET;
				send_packet.m_id = rc2.second->m_id;
				send_packet.m_pos = rc2.second->m_player->m_Pos;

				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_SPAWN_PACKET" << endl;
			}

			for (int i = 0; i < (int)GROUP_TYPE::UI; ++i) {
				if (i == (int)GROUP_TYPE::STRUCTURE) continue;
				if (i == (int)GROUP_TYPE::BULLET) continue;
				if (i == (int)GROUP_TYPE::PLAYER) continue;
				for (auto& object : CGameScene::m_objects[i]) {
					SC_ADD_PACKET send_packet;
					send_packet.m_size = sizeof(SC_ADD_PACKET);
					send_packet.m_type = PACKET_TYPE::P_SC_ADD_PACKET;
					send_packet.m_id = object.second->m_id;
					send_packet.m_grouptype = object.second->m_grouptype;
					send_packet.m_pos = object.second->m_Pos;
					send_packet.m_rota = object.second->m_Rota;
					send_packet.m_sca = object.second->m_Sca;
					strcpy_s(send_packet.m_modelName, object.second->m_modelname.c_str());

					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_NPC_ADD_PACKET" << endl;

				}
			}

			// Drop Items
			for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::UI]) {
				string name = reinterpret_cast<CItem*>(object.second)->m_ItemName;
				SC_DROPED_ITEM send_packet;
				send_packet.m_size = sizeof(SC_ADD_PACKET);
				send_packet.m_type = PACKET_TYPE::P_SC_DROPED_ITEM;
				send_packet.m_pos = object.second->m_Pos;
				strcpy_s(send_packet.m_itemName, name.c_str());

				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_DROPED_ITEM" << endl;
			}

			for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::BOX]) {
				CBox* box = reinterpret_cast<CBox*>(object.second);
				for (auto name : box->items) {
					SC_NPC_EXCHANGE_PACKET send_packet;
					send_packet.m_size = sizeof(SC_NPC_EXCHANGE_PACKET);
					send_packet.m_type = P_SC_NPC_EXCHANGE_PACKET;
					send_packet.m_id = box->m_id;
					send_packet.m_itemType = 2;
					strcpy_s(send_packet.m_itemName, name.c_str());
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_NPC_EXCHANGE_PACKET : need" << endl;
				}
			}


			for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::ONCE_ITEM]) {
				COnceItem* item = reinterpret_cast<COnceItem*>(object.second);
				for (auto name : item->items) {
					SC_NPC_EXCHANGE_PACKET send_packet;
					send_packet.m_size = sizeof(SC_NPC_EXCHANGE_PACKET);
					send_packet.m_type = P_SC_NPC_EXCHANGE_PACKET;
					send_packet.m_id = item->m_id;
					send_packet.m_itemType = 3;
					strcpy_s(send_packet.m_itemName, name.c_str());
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_NPC_EXCHANGE_PACKET : need" << endl;
				}
			}

			for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::NPC]) {
				CNPC* npc = reinterpret_cast<CNPC*>(object.second);
				for (auto name : npc->m_needs) {
					SC_NPC_EXCHANGE_PACKET send_packet;
					send_packet.m_size = sizeof(SC_NPC_EXCHANGE_PACKET);
					send_packet.m_type = P_SC_NPC_EXCHANGE_PACKET;
					send_packet.m_id = npc->m_id;
					send_packet.m_itemType = 0;
					strcpy_s(send_packet.m_itemName, name.c_str());
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_NPC_EXCHANGE_PACKET : need" << endl;
				}
				for (auto name : npc->m_outputs) {
					SC_NPC_EXCHANGE_PACKET send_packet;
					send_packet.m_size = sizeof(SC_NPC_EXCHANGE_PACKET);
					send_packet.m_type = P_SC_NPC_EXCHANGE_PACKET;
					send_packet.m_id = npc->m_id;
					send_packet.m_itemType = 1;
					strcpy_s(send_packet.m_itemName, name.c_str());
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_NPC_EXCHANGE_PACKET : output" << endl;
				}
			}
		}

		TIMER_EVENT ev{ chrono::system_clock::now(), EV_PHASE };
		CTimer::timer_queue.push(ev); 

		ev.event_id = EV_UPDATE;
		CTimer::timer_queue.push(ev);
}
	break;

	case PACKET_TYPE::P_CS_ATTACK_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		CS_ATTACK_PACKET* recv_packet = reinterpret_cast<CS_ATTACK_PACKET*>(_Packet);
		int demage = 0;
		if (!_Client->m_player->upperAnimationFinished) return;
		if (_Client->m_player->m_upAnimation != "Pistol_focus" && _Client->m_player->m_upAnimation != "Pistol_slowwalk")
		{
			// Attack 애니메이션 전송
			SC_ANIMATION_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_ANIMATION_PACKET;
			send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
			send_packet.m_id = _Client->m_id;
			send_packet.m_loop = false;
			send_packet.m_bone = 1;	// Upper
			_Client->m_player->upperAnimationFinished = false;
			switch (_Client->m_player->m_Weapon)
			{
			case 0:
				strcpy_s(send_packet.m_key, "Punch");
				demage = 5;
				break;
			case 1:
				strcpy_s(send_packet.m_key, "Pistol_shoot");
				break;
			case 2:
				strcpy_s(send_packet.m_key, "Attack_onehand");
				demage = 10;
				break;
			}
			_Client->m_player->lastAnimation = send_packet.m_key;
			TIMER_EVENT ev{ chrono::system_clock::now() + CGameScene::m_animations["hugo_idle"][string(send_packet.m_key)],
			EV_ANIMATION,_Client->m_id,(int)GROUP_TYPE::PLAYER, 1, send_packet.m_key };
			CTimer::timer_queue.push(ev);
			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
			}
		}

		// Pistol 일때 Bullet 생성
		if (_Client->m_player->m_Weapon == 1)
		{
			CObject* Bullet = CObject::Load("Bullet");
			Bullet->m_Pos = recv_packet->m_pos;
			Bullet->m_Rota = XMFLOAT3(0, recv_packet->m_angle, 0);
			Bullet->m_Vec = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(Bullet->m_Rota));
			Bullet->m_Velocity = 40.0f;
			CBullet* bullet = reinterpret_cast<CBullet*>(Bullet);
			bullet->m_lastTime = chrono::system_clock::now();
			bullet->m_id = bullet_id++;
			CGameScene::m_objects[(int)GROUP_TYPE::BULLET][bullet->m_id] = Bullet;
			
 			SC_ADD_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ADD_PACKET);
			send_packet.m_type = P_SC_ADD_PACKET;
			send_packet.m_id = bullet->m_id;
			send_packet.m_grouptype = (int)GROUP_TYPE::BULLET;
			send_packet.m_pos = Bullet->m_Pos;
			send_packet.m_rota = Bullet->m_Rota;
			send_packet.m_sca = XMFLOAT3(1, 1, 1);
			strcpy_s(send_packet.m_modelName, "Bullet");
			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_BULLET_ADD_PACKET" << endl;
			}
		}

		// Axe 일때 나무에 부딪히면 wood 반환
		BoundingBox bounding;
		bounding.Center = XMFLOAT3(0, 3, 0);
		bounding.Extents = XMFLOAT3(3, 3, 3);
		XMFLOAT3 look = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, recv_packet->m_angle, 0)));
		XMFLOAT3 attack = Vector3::Add(_Client->m_player->m_Pos, Vector3::ScalarProduct(look, 3.0f));
		XMFLOAT4X4 matrix = Matrix4x4::Identity();
		matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(attack));
		bounding.Transform(bounding, XMLoadFloat4x4(&matrix));

		if (_Client->m_player->m_Weapon == 2) {
			for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::STRUCTURE]) {
				if (object.second->m_modelname != "PP_Tree_02" && object.second->m_modelname != "dead_tree_a") continue;
				if (bounding.Intersects(object.second->m_boundingBox))
				{
					SC_PICKUP_PACKET send_packet;
					send_packet.m_size = sizeof(send_packet);
					send_packet.m_type = P_SC_PICKUP_PACKET;
					strcpy_s(send_packet.m_itemName, "wood");
					_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_PICKUP_PACKET" << endl;
				}
			}
		}

		// 몬스터에 닿았을 때 몬스터 체력 변화
		vector<int> deleteMonsters;
		for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::MONSTER]) {
			if (bounding.Intersects(object.second->m_boundingBox))
			{
				CMonster* monster = reinterpret_cast<CMonster*>(object.second);
				if (monster->m_dead && _Client->m_player->m_Weapon != 1) {
					if (monster->m_meet-- > 0) {
						SC_PICKUP_PACKET send_packet;
						send_packet.m_size = sizeof(send_packet);
						send_packet.m_type = P_SC_PICKUP_PACKET;
						strcpy_s(send_packet.m_itemName, "fish_meet");
						_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
						deleteMonsters.push_back(monster->m_id);
					}
					continue;
				}

				monster->m_Velocity = 0;
				SC_VELOCITY_CHANGE_PACKET v_send_packet;
				v_send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
				v_send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
				v_send_packet.m_id = monster->m_id;
				v_send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				v_send_packet.m_vel = monster->m_Velocity;
				v_send_packet.m_pos = monster->m_Pos;
				v_send_packet.m_look = monster->m_Angle;
				cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&v_send_packet));
				}

				monster->m_Health -= demage * 3;

				SC_ANIMATION_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_ANIMATION_PACKET;
				send_packet.m_grouptype = (int)GROUP_TYPE::MONSTER;
				send_packet.m_id = monster->m_id;
				send_packet.m_loop = false;
				send_packet.m_bone = 0;			// Root

				if (monster->m_Health <= 0) {
					// Monster 사망
					monster->m_dead = true;
					strcpy_s(send_packet.m_key, "Dead");
				}
				else
				{
					monster->m_stateMachine->ChangeState(CMonsterAttackedState::GetInstance());
				}

				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_ANIMATION_PACKET" << endl;
				}
			}
		}

		for (auto& monster : deleteMonsters) {
			CGameScene::m_objects[(int)GROUP_TYPE::MONSTER].erase(monster);
			SC_DELETE_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_DELETE_PACKET;
			send_packet.m_groupType = (int)GROUP_TYPE::MONSTER;
			send_packet.m_itemID = monster;
			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_DELETE_PACKET" << endl;
			}
		}

		// Player와 닿았을 때
		for (auto& object : RemoteClient::m_remoteClients) {
			if (object.second->m_player->m_dead) continue;
			if (!object.second->m_ingame) continue;
			if (object.second->m_id == _Client->m_id) continue;
			if (bounding.Intersects(object.second->m_player->m_boundingBox))
			{
				object.second->m_player->m_Health -= demage;
				cout << object.first << " : Health " << object.second->m_player->m_Health << endl;

				SC_HEALTH_CHANGE_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_HEALTH_CHANGE_PACKET;
				send_packet.m_id = object.second->m_id;
				send_packet.m_groupType = (int)GROUP_TYPE::PLAYER;
				send_packet.m_health = object.second->m_player->m_Health;
				for (auto& rc : RemoteClient::m_remoteClients) {
					if (!rc.second->m_ingame) continue;
					rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
				}
				if (object.second->m_player->m_Health <= 0) {
					SC_ANIMATION_PACKET send_packet;
					send_packet.m_size = sizeof(send_packet);
					send_packet.m_type = P_SC_ANIMATION_PACKET;
					send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
					send_packet.m_id = object.second->m_id;
					send_packet.m_loop = false;
					send_packet.m_bone = 0;	// Root
					strcpy_s(send_packet.m_key, "Death");
					for (auto& rc : RemoteClient::m_remoteClients) {
						if (!rc.second->m_ingame) continue;
						rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
						cout << " >> send ) SC_HEALTH_CHANGE_PACKET" << endl;
					}
					object.second->m_player->m_dead = true;
					CGameScene::m_Rank[CGameScene::m_nowRank--] = object.second->m_id;
				}
			}
		}
	}
	break;

	case PACKET_TYPE::P_CS_EXCHANGE_DONE_PACKET:
	{
		CS_EXCHANGE_DONE_PACKET* recv_packet = reinterpret_cast<CS_EXCHANGE_DONE_PACKET*>(_Packet);
		CNPC* npc = reinterpret_cast<CNPC*>(CGameScene::m_objects[(int)GROUP_TYPE::NPC][recv_packet->m_npc_id]);
		npc->m_standBy_id = _Client->m_id;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			SC_EXCHANGE_DONE_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_EXCHANGE_DONE_PACKET;
			send_packet.m_npc_id = recv_packet->m_npc_id;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_EXCHANGE_DONE_PACKET" << endl;
		}
	}
	break;

	case PACKET_TYPE::P_CS_PLAYER_RBUTTON_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		CS_PLAYER_RBUTTON_PACKET* recv_packet = reinterpret_cast<CS_PLAYER_RBUTTON_PACKET*>(_Packet);
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			SC_PLAYER_RBUTTON_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_PLAYER_RBUTTON_PACKET;
			send_packet.m_angle = recv_packet->m_angle;
			send_packet.m_id = _Client->m_id;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_EXCHANGE_DONE_PACKET" << endl;

		}
	}
	break;

	case PACKET_TYPE::P_CS_TAKEOUT_PACKET:
	{
		CS_TAKEOUT_PACKET* recv_packet = reinterpret_cast<CS_TAKEOUT_PACKET*>(_Packet);

		CObject* object = CGameScene::m_objects[recv_packet->m_groupType][recv_packet->m_itemID];

		CBox* box = reinterpret_cast<CBox*>(object);
		box->items.clear();

		if (recv_packet->m_groupType == (int)GROUP_TYPE::ONCE_ITEM)
		{
			for (auto& rc : RemoteClient::m_remoteClients) {
				if (!rc.second->m_ingame) continue;
				SC_DELETE_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_DELETE_PACKET;
				send_packet.m_groupType = recv_packet->m_groupType;
				send_packet.m_itemID = recv_packet->m_itemID;
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_DELETE_PACKET" << endl;
			}
		}
		else if (recv_packet->m_groupType == (int)GROUP_TYPE::BOX)
		{
			SC_TAKEOUT_PACKET send_packet;
			send_packet.m_size = sizeof(send_packet);
			send_packet.m_type = P_SC_TAKEOUT_PACKET;
			send_packet.m_itemID = recv_packet->m_itemID;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_TAKEOUT_PACKET" << endl;
		}
	}
	break;

	case PACKET_TYPE::P_CS_WEAPON_CHANGE_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		CS_WEAPON_CHANGE_PACKET* recv_packet = reinterpret_cast<CS_WEAPON_CHANGE_PACKET*>(_Packet);

		_Client->m_player->m_Weapon = recv_packet->m_weapon;
		SC_WEAPON_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_WEAPON_CHANGE_PACKET;
		send_packet.m_weapon = _Client->m_player->m_Weapon;
		send_packet.m_id = _Client->m_id;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
	break;

	case PACKET_TYPE::P_CS_DROPED_ITEM:
	{
		CS_DROPED_ITEM* recv_packet = reinterpret_cast<CS_DROPED_ITEM*>(_Packet);

		int itemid = item_id++;
		CItem* object = new CItem();
		object->m_id = itemid;
		object->m_Pos = _Client->m_player->m_Pos;
		object->m_Pos.y = CGameScene::OnGetHeight(object->m_Pos.x, object->m_Pos.z);
		object->m_ItemName = string(recv_packet->m_itemName);
		CGameScene::m_objects[(int)GROUP_TYPE::UI][itemid] = object;

		SC_DROPED_ITEM send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_DROPED_ITEM;
		send_packet.m_pos = object->m_Pos;
		send_packet.m_itemID = itemid;
		strcpy_s(send_packet.m_itemName, object->m_ItemName.c_str());
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
	break;

	case PACKET_TYPE::P_CS_PICKUP_PACKET:
	{
		if (_Client->m_player->m_dead) return;
		vector<int> deleteID;
		
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
		send_packet.m_id = _Client->m_id;
		send_packet.m_loop = false;
		send_packet.m_bone = 0;	// Upper
		strcpy_s(send_packet.m_key, "Gathering");
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_ANIMATION_PACKET" << endl;
		}
		_Client->m_player->lastAnimation = send_packet.m_key;
		TIMER_EVENT ev{ chrono::system_clock::now() + CGameScene::m_animations["hugo_idle"][string(send_packet.m_key)], 
			EV_ANIMATION, _Client->m_id, (int)GROUP_TYPE::PLAYER, 0, send_packet.m_key };
		CTimer::timer_queue.push(ev);

		for (auto& object : CGameScene::m_objects[(int)GROUP_TYPE::UI]) {
			if (CGameScene::can_see(_Client->m_player->m_Pos, object.second->m_Pos, 3)) {
				{
					SC_DELETE_PACKET send_packet;
					send_packet.m_size = sizeof(send_packet);
					send_packet.m_type = P_SC_DELETE_PACKET;
					send_packet.m_groupType = (int)GROUP_TYPE::UI;
					send_packet.m_itemID = object.second->m_id;
					for (auto& rc : RemoteClient::m_remoteClients) {
						if (!rc.second->m_ingame) continue;
						rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
						cout << " >> send ) SC_DELETE_PACKET" << endl;
					}
				}

				{
					SC_PICKUP_PACKET send_packet;
					send_packet.m_size = sizeof(send_packet);
					send_packet.m_type = P_SC_PICKUP_PACKET;
					strcpy_s(send_packet.m_itemName, reinterpret_cast<CItem*>(object.second)->m_ItemName.c_str());
					_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
					cout << " >> send ) SC_PICKUP_PACKET" << endl;
				}
				deleteID.push_back(object.second->m_id);
			}
		}
		for (auto id : deleteID) {
			CGameScene::m_objects[(int)GROUP_TYPE::UI].erase(id);
		}
	}
	break;

	case PACKET_TYPE::P_CS_RELOAD_PACKET:
	{
		// Attack 애니메이션 전송
		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(send_packet);
		send_packet.m_type = P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
		send_packet.m_id = _Client->m_id;
		send_packet.m_loop = false;
		send_packet.m_bone = 1;	// Upper
		strcpy_s(send_packet.m_key, "Reload");
		_Client->m_player->upperAnimationFinished = false;
		_Client->m_player->lastAnimation = send_packet.m_key;

		TIMER_EVENT ev{ chrono::system_clock::now() + CGameScene::m_animations["hugo_idle"][string(send_packet.m_key)],
				EV_ANIMATION, _Client->m_id, (int)GROUP_TYPE::PLAYER, 1, send_packet.m_key };
		CTimer::timer_queue.push(ev);
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ingame) continue;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> send ) SC_ANIMATION_PACKET" << endl;
		}
	}
	break;

	default:
		break;
	}

}
void CloseServer()
{
	lock_guard<recursive_mutex> lock_accept(mx_accept);
	// i/o 완료 체크 & 소켓 닫기
	listenSocket->Close();
	{
		lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);

		for (auto i : RemoteClient::m_remoteClients)
			i.second->m_tcpConnection.Close();

		// 정리 중
		cout << " >> Shutting down the server....\n";
		while (RemoteClient::m_remoteClients.size() > 0) {
			// I/O completion이 없는 상태의 RemoteClient를 제거
			for (auto i = RemoteClient::m_remoteClients.begin(); i != RemoteClient::m_remoteClients.end(); ++i) {
				if (!i->second->m_tcpConnection.m_isReadOverlapped)
					RemoteClient::m_remoteClients.erase(i);
			}

			// I/O completion이 발생 시 더 이상 Overlapped I/O를 걸지 말고 정리 신호 Flag.
			IocpEvents readEvents;
			Iocp::iocp.Wait(readEvents, 100);

			// 받은 이벤트 각각을 처리합니다.
			for (int i = 0; i < readEvents.m_eventCount; i++) {
				auto& readEvent = readEvents.m_events[i];
				if (readEvent.lpCompletionKey == 0)  {
					listenSocket->m_isReadOverlapped = false;
				}
				else {
					shared_ptr<RemoteClient> remoteClient = RemoteClient::m_remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient) {
						remoteClient->m_tcpConnection.m_isReadOverlapped = false;
					}
				}
			}
		}
	}
	std::cout << " >> Server End.\n";
}