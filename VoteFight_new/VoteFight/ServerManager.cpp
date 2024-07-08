#include "pch.h"

#include "./ImaysNet/ImaysNet.h"
#include "./ImaysNet/PacketQueue.h"

#include "ServerManager.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "PlayerStates.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "Scene.h"
#include "Player.h"
#include "StateMachine.h"
#include "UI.h"
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"
#include "GameFramework.h"
#include "Camera.h"

#include "Object.h"
#include "LoginScene.h"
#pragma comment(lib, "WS2_32.LIB")

// 서버 IP
char* CServerManager::m_SERVERIP;
//char* CServerManager::m_SERVERIP = "127.0.0.1";

// 재귀적 mutex
recursive_mutex CServerManager::m_mutex;

// 소켓 객체
shared_ptr<Socket> CServerManager::m_tcpSocket;

// 클라이언트 ID
int		CServerManager::m_id{ -1 };
bool	CServerManager::m_isLogin{ false };

// 소켓 수신 콜백 함수
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	// 수신된 데이터를 갖고 오기 위한 버퍼와 수신된 데이터 길이
	char* recv_buf = reinterpret_cast<EXP_OVER*>(recv_over)->m_buf;
	int recv_buf_Length = num_bytes;

	// cout << " >> Packet Size - " << (int)recv_buf[0] << endl;
	// cout << " >> num_bytes - " << num_bytes << endl;
	// cout << " >> Packet Type - " << (int)recv_buf[1] << endl;

	{ 
		// 수신된 데이터 처리
		int remain_data = recv_buf_Length + CServerManager::m_tcpSocket->m_prev_remain;
		while (remain_data > 0) {    					// 패킷의 크기를 확인하여 처리
			unsigned char packet_size = recv_buf[0];	// 패킷 크기 저장
		
			if (packet_size > remain_data)				// 패킷 크기가 남은 데이터보다 크면 종료
				break;
			else if (packet_size == 0) {				// 패킷 크기가 0이어도 종료
				remain_data = 0;
				break;
			}

			CServerManager::PacketProcess(recv_buf);   // 패킷 처리

			recv_buf += packet_size;					// 다음 패킷으로 이동
			remain_data -= packet_size;					// 남은 데이터 갱신
		}

		// 남은 데이터 저장
		CServerManager::m_tcpSocket->m_prev_remain = remain_data;

		// 남은 데이터가 있으면 새 오버랩 구조체에 복사
		if (remain_data > 0) {
			memcpy(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf, recv_buf, remain_data);
		}
	}

	// 남은 데이터 이후의 메모리 초기화
	memset(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf + CServerManager::m_tcpSocket->m_prev_remain, 0,
		sizeof(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf) - CServerManager::m_tcpSocket->m_prev_remain);
	
	// 오버랩 구조체 초기화
	memset(&CServerManager::m_tcpSocket->m_recvOverlapped.m_wsa_over, 0, sizeof(CServerManager::m_tcpSocket->m_recvOverlapped.m_wsa_over));
	
	// 다음 데이터 수신
	CServerManager::Do_Recv();
}

// 송신 콜백 함수
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	// 송신이 완료된 후 호출되는 콜백 함수
	// 오버랩 구조체 삭제하여 메모리 누수 방지
	delete reinterpret_cast<EXP_OVER*>(send_over);
}

void CServerManager::ConnectServer(string server_s)	// 서버 연결 함수
{
	m_tcpSocket = make_shared<Socket>(SocketType::Tcp);
	m_SERVERIP = new char[server_s.size() + 1];
	strncpy(m_SERVERIP, server_s.c_str(), server_s.size());
	m_SERVERIP[server_s.size()] = '\0';

	m_tcpSocket->Bind(Endpoint::Any);
	CServerManager::Connetion();		// 연결

	CServerManager::Do_Recv();			// 데이터 수신 시작
}

void CServerManager::Tick()				//주기적인 작업 실행 함수
{
	SleepEx(0, true);					

	// 패킷 큐 확인 및 서버로 전송
	if (PacketQueue::m_SendQueue.empty() || m_tcpSocket->m_fd == INVALID_SOCKET)
		return;

	while (!PacketQueue::m_SendQueue.empty()) {
		//  패킷 큐에서 패킷을 가져와 송신
		char* send_buf = PacketQueue::m_SendQueue.front();
		int buf_size{};
		while (1) {
			// 전송할 패킷의 크기 계산
			if (buf_size + send_buf[buf_size] > MAX_BUFSIZE_CLIENT || send_buf[buf_size] == 0)
				break;
			buf_size += send_buf[buf_size];
		}
		Do_Send(send_buf, buf_size);		// 데이터 송신
		PacketQueue::PopSendPacket();		// 송신한 패킷 제거
	}
}

void CServerManager::Connetion()			// Connect 함수
{
	m_tcpSocket->Connect(Endpoint(m_SERVERIP, SERVER_PORT));
}

void CServerManager::Do_Recv()				// 데이터 수신 함수
{
	m_tcpSocket->m_readFlags = 0;			// 소켓에 대한 읽기 플래그 초기화
	
	// 오버랩 구조체 초기화
	ZeroMemory(&m_tcpSocket->m_recvOverlapped.m_wsa_over, sizeof(m_tcpSocket->m_recvOverlapped.m_wsa_over));
	
	// 수신 버퍼 설정
	m_tcpSocket->m_recvOverlapped.m_wsa_buf.len = MAX_SOCKBUF - m_tcpSocket->m_prev_remain;
	m_tcpSocket->m_recvOverlapped.m_wsa_buf.buf = m_tcpSocket->m_recvOverlapped.m_buf + m_tcpSocket->m_prev_remain;

	// 비동기 수신 시작
	WSARecv(m_tcpSocket->m_fd, &(m_tcpSocket->m_recvOverlapped.m_wsa_buf), 1, 0,
		&m_tcpSocket->m_readFlags, &(m_tcpSocket->m_recvOverlapped.m_wsa_over), recv_callback);
}

void CServerManager::Do_Send(const char* _buf, short _buf_size)		// 데이터 송신 함수
{
	EXP_OVER* send_over = new EXP_OVER(_buf, _buf_size);			// // 송신용 오버랩 구조체 생성
	
	// 비동기 송신 시작
	WSASend(m_tcpSocket->m_fd, &send_over->m_wsa_buf, 1, 0, 0,
		&send_over->m_wsa_over, send_callback);
}

void CServerManager::PacketProcess(char* _Packet)	// 패킷 처리 함수
{
	// Packet Types Processing
	switch (_Packet[1])
	{

	case PACKET_TYPE::P_SC_LOGIN_OK_PACKET:
	{
		SC_LOGIN_OK_PACKET* recv_packet = reinterpret_cast<SC_LOGIN_OK_PACKET*>(_Packet);
		cout << "SC_LOGIN_OK_PACKET" << endl;
		CLoginScene* loginscene = reinterpret_cast<CLoginScene*>(CSceneManager::GetInstance()->GetCurrentScene());
		string name = loginscene->user_name;
		CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::GAME);
		
		// 오브젝트 로드
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* object = CObject::Load("hugo_idle");
		CPlayer* player = reinterpret_cast<CPlayer*>(object);
		player->m_name = name;
		scene->m_myPlayer = player;

		// ID 설정
		CServerManager::m_id = recv_packet->m_id;
		CGameFramework::GetInstance()->my_id = recv_packet->m_id;
		player->m_id = recv_packet->m_id;

		// 위치 설정
		CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
		transform->SetPosition(recv_packet->m_vec);
		//object->SetTerrainY(CSceneManager::GetInstance()->GetCurrentScene());

		CAnimator* animator = reinterpret_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->SetWeight("Idle", 1.0f);
		animator->Play("Idle", true);

		CSceneManager::GetInstance()->GetCurrentScene()->AddObject(GROUP_TYPE::PLAYER, object);
		player->Init();

		// State Machine 설정
		player->CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
		CStateMachine* statemachine = reinterpret_cast<CStateMachine*>(object->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		statemachine->SetCurrentState(CPlayerIdleState::GetInstance());

		// 씬에 추가
		scene->AddObjectID(player, recv_packet->m_id);
		scene->oldXCell = scene->oldZCell = -1;

		// 카메라 타겟 세팅
		CCameraManager::GetInstance()->GetMainCamera()->SetTarget(object);

		cout << "Clinet ID - " << player->m_id << endl;
		break;
	}

	case  PACKET_TYPE::P_SC_ADD_PACKET:
	{
		SC_ADD_PACKET* recv_packet = reinterpret_cast<SC_ADD_PACKET*>(_Packet);
		cout << "SC_ADD_PLAYER" << endl;

		if (CGameFramework::GetInstance()->my_id == recv_packet->m_id) {
			cout << "Error: Add my ID" << endl;
			return;
		}
		// 오브젝트 로드

		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* object = CObject::Load("hugo_idle");
		CPlayer* player = reinterpret_cast<CPlayer*>(object);
		strcpy_s(recv_packet->m_name, player->m_name.c_str());

		// ID 설정
		player->m_id = recv_packet->m_id;

		// 위치 설정
		CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
		transform->SetPosition(recv_packet->m_vec);

		CAnimator* animator = reinterpret_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->SetWeight("Idle", 1.0f);
		animator->Play("Idle", true);

		player->Init();

		// 씬에 추가
		scene->AddObjectID(player, recv_packet->m_id);
		scene->AddObject(GROUP_TYPE::PLAYER, player);
		}
	break;

	case PACKET_TYPE::P_SC_WALK_ENTER_INFO_PACKET:
	{
		SC_WALK_ENTER_INFO_PACKET* recv_packet = reinterpret_cast<SC_WALK_ENTER_INFO_PACKET*>(_Packet);
		 cout << "SC_WALK_ENTER_INFO_PACKET" << endl;

		string ani_key = recv_packet->m_key;
		float max_sed = recv_packet->m_maxSpeed;
		float velocity = recv_packet->m_vel;
		
		// 플레이어 객체를 얻어와야 함.
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CPlayer* player = scene->m_myPlayer;

		CAnimator* animator = static_cast<CAnimator*>(player->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->Play(ani_key, true);

		CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));
		CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

		rigidBody->SetMaxSpeedXZ(max_sed);
		rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), velocity * DT));
		break;
	}

	case PACKET_TYPE::P_SC_MOVE_V_PACKET:
	{
		SC_MOVE_V_PACKET* recv_packet = reinterpret_cast<SC_MOVE_V_PACKET*>(_Packet);
		vector<CObject*> objects = CSceneManager::GetInstance()->GetCurrentScene()->GetGroupObject(GROUP_TYPE::PLAYER);

		for (auto& p : objects) {
			CPlayer* player = reinterpret_cast<CPlayer*>(p);
			if (CGameFramework::GetInstance()->my_id == recv_packet->m_id) continue;
			if (player->m_id != recv_packet->m_id) continue;

			CTransform* net_transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
			CStateMachine* net_stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));

			net_transform->SetPosition(XMFLOAT3(recv_packet->m_vec.x, recv_packet->m_vec.y, recv_packet->m_vec.z));
			net_transform->SetRotation(XMFLOAT3(recv_packet->m_rota.x, recv_packet->m_rota.y, recv_packet->m_rota.z));

			// cout << "ID - " << player->m_id << ", xPos - " << recv_packet->m_vec.x << ", yPos - " << recv_packet->m_vec.y << ", zPos - " << recv_packet->m_vec.z << endl;
		}
		break;
	}

	case PACKET_TYPE::P_SC_POS_PACKET:
	{
		SC_POS_PACKET* recv_packet = reinterpret_cast<SC_POS_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* obj = scene->GetIDObject(recv_packet->m_id);
		if (obj != nullptr) {
			CTransform* transform = static_cast<CTransform*>(obj->GetComponent(COMPONENT_TYPE::TRANSFORM));
			transform->SetPosition(recv_packet->m_pos);
			transform->SetRotation(XMFLOAT3(0, recv_packet->m_rota, 0));
		}
	}
	break;

	case PACKET_TYPE::P_SC_ANIMATION:
	{
		SC_ANIMATION_PACKET* recv_packet = reinterpret_cast<SC_ANIMATION_PACKET*>(_Packet);
		cout << "SC_ANIMATION_PACKET" << endl;

		string ani_key = recv_packet->m_key;
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();

		CObject* obj = scene->GetIDObject(recv_packet->m_id);
		CAnimator* animator = static_cast<CAnimator*>(obj->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->Play(ani_key, true);
	}
	break;

	default:
		break;
	}
}