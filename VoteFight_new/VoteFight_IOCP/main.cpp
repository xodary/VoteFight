#pragma once
#include "pch.h"
#include "./ImaysNet/ImaysNet.h"
#include "RemoteClient.h"
#include "Timer.h"
#include "GameScene.h"
#include "NPC.h"
#include "Box.h"

volatile bool				stopServer = false;
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
int	CTimer::phase;
chrono::seconds phase_time[8] = { 150s, 90s,150s, 90s,120s, 60s,120s, 60s };

// Client 종료 처리 함수
void	ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient);
void	ProcessAccept();
void	PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet);
void	WorkerThread();
void	UpdatePos(OVERLAPPED_ENTRY& readEvent);
void	UpdatePhase(OVERLAPPED_ENTRY& readEvent);
void	CloseServer();

// 서버 프로그램 진입점
int main(int argc, char* argv[])
{
	CGameScene::LoadTerrain("HeightMap.bin");
	CGameScene::Load("ServerScene.bin");
	CGameScene::Load("GameScene.bin");

	CGameScene::NPCInitialize();

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
		lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
		RemoteClient::m_remoteClients.erase(_remoteClient.get());	// 클라이언트 제거

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

				if (IO_TYPE::IO_UPDATE == p_readOverlapped->m_ioType) {
					//cout << "Update" << endl;
					UpdatePos(readEvent);
					continue;
				}

				if (IO_TYPE::IO_PHASE == p_readOverlapped->m_ioType) {
					//cout << "Update" << endl;
					UpdatePhase(readEvent);
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

void UpdatePos(OVERLAPPED_ENTRY& readEvent)
{
	for (auto& client : RemoteClient::m_remoteClients) {
		if (client.second == nullptr || !client.second->m_ingame) continue;
		auto duration = chrono::system_clock::now() - client.second->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		XMFLOAT3 shift = Vector3::ScalarProduct(client.second->m_player->m_Vec, seconds * client.second->m_player->m_Velocity);
		client.second->m_player->m_Pos = Vector3::Add(client.second->m_player->m_Pos, shift);
		client.second->m_lastTime = chrono::system_clock::now();
		
		XMFLOAT4X4 matrix = Matrix4x4::Identity();
		//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Scale(client.second->m_player->m_Sca));
		//matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Rotation(client.second->m_player->m_Rota));
		matrix = Matrix4x4::Multiply(matrix, Matrix4x4::Translation(client.second->m_player->m_Pos));
		client.second->m_player->m_origin.Transform(client.second->m_player->m_boundingBox, XMLoadFloat4x4(&matrix));
		
		for (int i = 0; i < (int)GROUP_TYPE::COUNT; ++i) {
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
			if (rc.second == nullptr || !rc.second->m_ingame) continue;
			SC_POS_PACKET send_packet;
			send_packet.m_size = sizeof(SC_POS_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_POS_PACKET;
			//send_packet.m_grouptype = client.second->m_player->m_grouptype;
			send_packet.m_grouptype = (int)GROUP_TYPE::PLAYER;
			send_packet.m_id = client.second->m_id;
			send_packet.m_pos = client.second->m_player->m_Pos;
			//send_packet.m_rota = rc.second->m_player->m_Rota;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
	}
	TIMER_EVENT ev{ (RemoteClient*)readEvent.lpCompletionKey, chrono::system_clock::now() + 100ms, EV_UPDATE, 0 };
	CTimer::timer_queue.push(ev);
}

void UpdatePhase(OVERLAPPED_ENTRY& readEvent)
{
	for (auto& client : RemoteClient::m_remoteClients) {
		if (client.second == nullptr || !client.second->m_ingame) continue;
		auto duration = chrono::system_clock::now() - client.second->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		XMFLOAT3 shift = Vector3::ScalarProduct(client.second->m_player->m_Vec, seconds * client.second->m_player->m_Velocity);
		client.second->m_player->m_Pos = Vector3::Add(client.second->m_player->m_Pos, shift);
		client.second->m_lastTime = chrono::system_clock::now();

		SC_UPDATE_PHASE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_UPDATE_PHASE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_UPDATE_PHASE_PACKET;
		send_packet.m_phase = CTimer::phase;
		send_packet.m_time = phase_time[CTimer::phase];
		client.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));

		cout << "Phase " << CTimer::phase << endl;
	}
	TIMER_EVENT ev{ (RemoteClient*)readEvent.lpCompletionKey, chrono::system_clock::now() + phase_time[CTimer::phase++], EV_PHASE, 0 };
	CTimer::timer_queue.push(ev);
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

	case PACKET_TYPE::P_CS_WALK_ENTER_PACKET:
	{
		CS_WALK_ENTER_PACKET* recv_packet = reinterpret_cast<CS_WALK_ENTER_PACKET*>(_Packet);
		cout << "SC_WALK_ENTER_INFO_PACKET" << endl;

		SC_ANIMATION_PACKET send_packet;
		send_packet.m_size = sizeof(SC_ANIMATION_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_ANIMATION_PACKET;
		send_packet.m_grouptype = _Client->m_player->m_grouptype;
		send_packet.m_id = _Client->m_id;
		strcpy_s(send_packet.m_key, "Run");

		for (auto& rc : RemoteClient::m_remoteClients) {
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		cout << " >> send ) CS_WALK_ENTER_PACKET" << endl;
	}
	break;

	case PACKET_TYPE::P_CS_VELOCITY_CHANGE_PACKET:
	{
		CS_VELOCITY_CHANGE_PACKET* recv_packet = reinterpret_cast<CS_VELOCITY_CHANGE_PACKET*>(_Packet);

		XMFLOAT3 vector = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, recv_packet->m_angle, 0)));
		if (recv_packet->m_shift) _Client->m_player->m_Velocity = 15; else _Client->m_player->m_Velocity = 10;

		auto duration = chrono::system_clock::now() - _Client->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		cout << seconds << endl;
		XMFLOAT3 shift = Vector3::ScalarProduct(_Client->m_player->m_Vec, seconds * _Client->m_player->m_Velocity);
		_Client->m_player->m_Pos = Vector3::Add(_Client->m_player->m_Pos, shift);
		_Client->m_lastTime = chrono::system_clock::now();
		_Client->m_player->m_Vec = vector;
		_Client->m_player->m_Angle = recv_packet->m_angle;

		SC_VELOCITY_CHANGE_PACKET send_packet;
		send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
		send_packet.m_id = _Client->m_id;
		send_packet.m_angle = recv_packet->m_angle;
		send_packet.m_grouptype = _Client->m_player->m_grouptype;
		send_packet.m_vel = _Client->m_player->m_Velocity;
		send_packet.m_pos = _Client->m_player->m_Pos;
		cout << " >> send ) CS_VELOCITY_CHANGE_PACKET" << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		break;
	}

	case PACKET_TYPE::P_CS_STOP_PACKET:
	{
		CS_STOP_PACKET* recv_packet = reinterpret_cast<CS_STOP_PACKET*>(_Packet);
		auto duration = chrono::system_clock::now() - _Client->m_lastTime;
		auto seconds = chrono::duration_cast<std::chrono::duration<float>>(duration).count();
		cout << seconds << endl;
		XMFLOAT3 shift = Vector3::ScalarProduct(_Client->m_player->m_Vec, seconds * _Client->m_player->m_Velocity);
		_Client->m_player->m_Pos = recv_packet->m_pos;
		_Client->m_lastTime = chrono::system_clock::now();
		_Client->m_player->m_Velocity = 0;

		{
			SC_VELOCITY_CHANGE_PACKET send_packet;
			send_packet.m_size = sizeof(SC_VELOCITY_CHANGE_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET;
			send_packet.m_id = _Client->m_id;
			send_packet.m_grouptype = _Client->m_player->m_grouptype;
			send_packet.m_angle = _Client->m_player->m_Angle;
			send_packet.m_vel = _Client->m_player->m_Velocity;
			send_packet.m_pos = _Client->m_player->m_Pos;
			
			_Client->m_lastTime = chrono::system_clock::now();
			cout << " >> send ) SC_VELOCITY_CHANGE_PACKET" << endl;

			for (auto& rc : RemoteClient::m_remoteClients) {
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}

		{
			SC_ANIMATION_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ANIMATION_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_ANIMATION_PACKET;
			send_packet.m_grouptype = _Client->m_player->m_grouptype;
			send_packet.m_id = _Client->m_id;
			strcpy_s(send_packet.m_key, "Idle");
			cout << " >> send ) SC_ANIMATION_PACKET" << endl;

			for (auto& rc : RemoteClient::m_remoteClients) {
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}
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
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}

		if (RemoteClient::m_remoteClients.size() < MAX_PLAYER) return;
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (!rc.second->m_ready) {
				return;
			}
		}

		cout << " GameStart" << endl;
		{
			SC_GAMESTART_PACKET send_packet;
			send_packet.m_size = sizeof(SC_GAMESTART_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_GAMESTART_PACKET;

			for (auto& rc : RemoteClient::m_remoteClients) {
				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			}
		}
		cout << " >> send ) SC_GAMESTART_PACKET" << endl;

		XMFLOAT3 pos[3]{ {10, 0, 10},{380, 0, 16},{388, 0, 382} };
		for (auto& rc : RemoteClient::m_remoteClients) {
			int i = 0;
			for (auto& rc2 : RemoteClient::m_remoteClients) {	// other players
				rc2.second->m_player->m_Pos = pos[i++];

				SC_SPAWN_PACKET send_packet;
				send_packet.m_size = sizeof(send_packet);
				send_packet.m_type = P_SC_SPAWN_PACKET;
				send_packet.m_id = rc2.second->m_id;
				send_packet.m_pos = rc2.second->m_player->m_Pos;

				rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
				cout << " >> send ) SC_SPAWN_PACKET" << endl;
			}

			for (int i = 0; i < (int)GROUP_TYPE::COUNT; ++i) {
				if (i == (int)GROUP_TYPE::STRUCTURE) continue;
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

		EXP_OVER* exover = new EXP_OVER;
		exover->m_ioType = IO_TYPE::IO_PHASE;
		PostQueuedCompletionStatus(Iocp::iocp.m_hIocp, 1, 0, &exover->m_wsa_over);

		exover = new EXP_OVER;
		exover->m_ioType = IO_TYPE::IO_UPDATE;
		PostQueuedCompletionStatus(Iocp::iocp.m_hIocp, 1, 0, &exover->m_wsa_over);
	}
	break;

	case PACKET_TYPE::P_CS_ATTACK_PACKET:
	{
		CS_ATTACK_PACKET* recv_packet = reinterpret_cast<CS_ATTACK_PACKET*>(_Packet);
		//_Client->m_player->m_Pos
		//recv_packet->m_vec;
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