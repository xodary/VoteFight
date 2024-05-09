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
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"
#include "GameFramework.h"
#include "Camera.h"

#include "Object.h"
#pragma comment(lib, "WS2_32.LIB")

// 서버 IP
//char* CServerManager::SERVERIP;
char* CServerManager::m_SERVERIP = "127.0.0.1";

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

	cout << " >> Packet Size - " << (int)recv_buf[0] << endl;
	cout << " >> num_bytes - " << num_bytes << endl;
	cout << " >> Packet Type - " << (int)recv_buf[1] << endl;

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

void CServerManager::ConnectServer()	// 서버 연결 함수
{
	m_tcpSocket = make_shared<Socket>(SocketType::Tcp);

	/*std::cout << std::endl << " [ =========== Login =========== ] " << std::endl << std::endl;

	std::cout << std::endl << "Input Connect Server IP (ex 100.xxx.xxx.xxx) : " << std::endl;
	std::string server_s;
	std::cin >> server_s;
	m_SERVERIP = new char[server_s.size() + 1];
	m_SERVERIP[server_s.size()] = '\0';
	strncpy(m_SERVERIP, server_s.c_str(), server_s.size());*/

	m_tcpSocket->Bind(Endpoint::Any);
	CServerManager::Connetion();		// 연결

	CS_LOGIN_PACKET send_packet;
	send_packet.m_size = sizeof(CS_LOGIN_PACKET);
	send_packet.m_type = PACKET_TYPE::P_CS_LOGIN_PACKET;
	PacketQueue::AddSendPacket(&send_packet);

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
		CServerManager::m_id = recv_packet->m_id;
		cout << "ClinetManager ID - " << CServerManager::m_id << endl;

		CObject* object = CObject::Load("hugo_idle");
		CPlayer* player = reinterpret_cast<CPlayer*>(object);
		dynamic_cast<CPlayer*>(player)->m_id = recv_packet->m_id;
		CSceneManager::GetInstance()->GetCurrentScene()->AddObject(GROUP_TYPE::STRUCTURE, object);
		CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
		transform->SetPosition(XMFLOAT3(0, 0, 0));
		break;
	}

	case  PACKET_TYPE::P_SC_ADD_PACKET: 
	{
		SC_ADD_PACKET* recv_packet = reinterpret_cast<SC_ADD_PACKET*>(_Packet);
		cout << "SC_ADD_PLAYER" << endl;

		/*if (CGameScene::m_CGameScene->m_otherPlayers.size() < 3) {
			CObject* object = CObject::Load("hugo_idle");
			CPlayer* player = reinterpret_cast<CPlayer*>(object);
			dynamic_cast<CPlayer*>(player)->m_id = recv_packet->m_id;
			CSceneManager::GetInstance()->GetCurrentScene()->AddObject(GROUP_TYPE::PLAYER, object);
			CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
			transform->SetPosition(XMFLOAT3(0, 0, 0));

			CGameScene::m_CGameScene->m_otherPlayers.push_back(player);
		} else {
			cout << " Max Player!! " << endl;
		}*/
		break;
	}

	case PACKET_TYPE::P_SC_WALK_ENTER_INFO_PACKET:
	{
		SC_WALK_ENTER_INFO_PACKET* recv_packet = reinterpret_cast<SC_WALK_ENTER_INFO_PACKET*>(_Packet);
		// cout << "SC_WALK_ENTER_INFO_PACKET" << endl;

		/*string ani_key = recv_packet->m_key;
		float max_sed = recv_packet->m_maxSpeed;
		float velocity = recv_packet->m_vel;
		
		// 플레이어 객체를 얻어와야 함. (실제 함수에선 객체를 인자로 받지 않?)
		// CPlayer* player = GameFramework::MainGameFramework->m_pPlayer;

		CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->Play(ani_key, true);

		CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
		CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

		rigidBody->SetMaxSpeedXZ(max_sed);
		rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), velocity * DT));*/
		break;
	}
	default:
		break;
	}
}