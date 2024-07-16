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
#include "TimeManager.h"
#include "Object.h"
#include "LoginScene.h"
#include "SelectScene.h"
#include "NPC.h"
#include "Box.h"
#pragma comment(lib, "WS2_32.LIB")

// ���� IP
char* CServerManager::m_SERVERIP;
//char* CServerManager::m_SERVERIP = "127.0.0.1";

// ����� mutex
recursive_mutex CServerManager::m_mutex;

// ���� ��ü
shared_ptr<Socket> CServerManager::m_tcpSocket;

// Ŭ���̾�Ʈ ID
int		CServerManager::m_id{ -1 };
bool	CServerManager::m_isLogin{ false };

// ���� ���� �ݹ� �Լ�
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	// ���ŵ� �����͸� ���� ���� ���� ���ۿ� ���ŵ� ������ ����
	char* recv_buf = reinterpret_cast<EXP_OVER*>(recv_over)->m_buf;
	int recv_buf_Length = num_bytes;

	// cout << " >> Packet Size - " << (int)recv_buf[0] << endl;
	// cout << " >> num_bytes - " << num_bytes << endl;
	// cout << " >> Packet Type - " << (int)recv_buf[1] << endl;

	{ 
		// ���ŵ� ������ ó��
		int remain_data = recv_buf_Length + CServerManager::m_tcpSocket->m_prev_remain;
		while (remain_data > 0) {    					// ��Ŷ�� ũ�⸦ Ȯ���Ͽ� ó��
			unsigned char packet_size = recv_buf[0];	// ��Ŷ ũ�� ����
		
			if (packet_size > remain_data)				// ��Ŷ ũ�Ⱑ ���� �����ͺ��� ũ�� ����
				break;
			else if (packet_size == 0) {				// ��Ŷ ũ�Ⱑ 0�̾ ����
				remain_data = 0;
				break;
			}

			CServerManager::PacketProcess(recv_buf);   // ��Ŷ ó��

			recv_buf += packet_size;					// ���� ��Ŷ���� �̵�
			remain_data -= packet_size;					// ���� ������ ����
		}

		// ���� ������ ����
		CServerManager::m_tcpSocket->m_prev_remain = remain_data;

		// ���� �����Ͱ� ������ �� ������ ����ü�� ����
		if (remain_data > 0) {
			memcpy(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf, recv_buf, remain_data);
		}
	}

	// ���� ������ ������ �޸� �ʱ�ȭ
	memset(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf + CServerManager::m_tcpSocket->m_prev_remain, 0,
		sizeof(CServerManager::m_tcpSocket->m_recvOverlapped.m_buf) - CServerManager::m_tcpSocket->m_prev_remain);
	
	// ������ ����ü �ʱ�ȭ
	memset(&CServerManager::m_tcpSocket->m_recvOverlapped.m_wsa_over, 0, sizeof(CServerManager::m_tcpSocket->m_recvOverlapped.m_wsa_over));
	
	// ���� ������ ����
	CServerManager::Do_Recv();
}

// �۽� �ݹ� �Լ�
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag)
{
	// �۽��� �Ϸ�� �� ȣ��Ǵ� �ݹ� �Լ�
	// ������ ����ü �����Ͽ� �޸� ���� ����
	delete reinterpret_cast<EXP_OVER*>(send_over);
}

void CServerManager::ConnectServer(string server_s)	// ���� ���� �Լ�
{
	m_tcpSocket = make_shared<Socket>(SocketType::Tcp);
	m_SERVERIP = new char[server_s.size() + 1];
	strncpy(m_SERVERIP, server_s.c_str(), server_s.size());
	m_SERVERIP[server_s.size()] = '\0';

	m_tcpSocket->Bind(Endpoint::Any);
	CServerManager::Connetion();		// ����

	CServerManager::Do_Recv();			// ������ ���� ����
}

void CServerManager::Tick()				//�ֱ����� �۾� ���� �Լ�
{
	SleepEx(0, true);					

	// ��Ŷ ť Ȯ�� �� ������ ����
	if (PacketQueue::m_SendQueue.empty() || m_tcpSocket->m_fd == INVALID_SOCKET)
		return;

	while (!PacketQueue::m_SendQueue.empty()) {
		//  ��Ŷ ť���� ��Ŷ�� ������ �۽�
		char* send_buf = PacketQueue::m_SendQueue.front();
		int buf_size{};
		while (1) {
			// ������ ��Ŷ�� ũ�� ���
			if (buf_size + send_buf[buf_size] > MAX_BUFSIZE_CLIENT || send_buf[buf_size] == 0)
				break;
			buf_size += send_buf[buf_size];
		}
		Do_Send(send_buf, buf_size);		// ������ �۽�
		PacketQueue::PopSendPacket();		// �۽��� ��Ŷ ����
	}
}

void CServerManager::Connetion()			// Connect �Լ�
{
	m_tcpSocket->Connect(Endpoint(m_SERVERIP, SERVER_PORT));
}

void CServerManager::Do_Recv()				// ������ ���� �Լ�
{
	m_tcpSocket->m_readFlags = 0;			// ���Ͽ� ���� �б� �÷��� �ʱ�ȭ
	
	// ������ ����ü �ʱ�ȭ
	ZeroMemory(&m_tcpSocket->m_recvOverlapped.m_wsa_over, sizeof(m_tcpSocket->m_recvOverlapped.m_wsa_over));
	
	// ���� ���� ����
	m_tcpSocket->m_recvOverlapped.m_wsa_buf.len = MAX_SOCKBUF - m_tcpSocket->m_prev_remain;
	m_tcpSocket->m_recvOverlapped.m_wsa_buf.buf = m_tcpSocket->m_recvOverlapped.m_buf + m_tcpSocket->m_prev_remain;

	// �񵿱� ���� ����
	WSARecv(m_tcpSocket->m_fd, &(m_tcpSocket->m_recvOverlapped.m_wsa_buf), 1, 0,
		&m_tcpSocket->m_readFlags, &(m_tcpSocket->m_recvOverlapped.m_wsa_over), recv_callback);
}

void CServerManager::Do_Send(const char* _buf, short _buf_size)		// ������ �۽� �Լ�
{
	EXP_OVER* send_over = new EXP_OVER(_buf, _buf_size);			// // �۽ſ� ������ ����ü ����
	
	// �񵿱� �۽� ����
	WSASend(m_tcpSocket->m_fd, &send_over->m_wsa_buf, 1, 0, 0,
		&send_over->m_wsa_over, send_callback);
}

void CServerManager::PacketProcess(char* _Packet)	// ��Ŷ ó�� �Լ�
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
		CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::SELECT);

		// ID ����
		CServerManager::m_id = recv_packet->m_id;
		CGameFramework::GetInstance()->my_id = recv_packet->m_id;

		cout << "Clinet ID - " << recv_packet->m_id << endl;
		break;
	}

	case PACKET_TYPE::P_SC_SPAWN_PACKET:	// Player
	{
		cout << "P_SC_SPAWN_PACKET" << endl;
		SC_SPAWN_PACKET* recv_packet = reinterpret_cast<SC_SPAWN_PACKET*>(_Packet);

		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* object = scene->GetIDObject(GROUP_TYPE::PLAYER, recv_packet->m_id);

		if (recv_packet->m_id == CGameFramework::GetInstance()->my_id) {
			// State Machine ����
			object->CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
			CStateMachine* statemachine = reinterpret_cast<CStateMachine*>(object->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
			statemachine->SetCurrentState(CPlayerIdleState::GetInstance());

			scene->oldXCell = scene->oldZCell = -1;
			CCameraManager::GetInstance()->GetMainCamera()->SetTarget(object);
		}
		// ��ġ ����
		CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
		transform->SetPosition(recv_packet->m_pos);
		int xNewCell = clamp((int)(recv_packet->m_pos.x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
		int zNewCell = clamp((int)(recv_packet->m_pos.z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);
		scene->ObjectListSector[zNewCell * SECTOR_RANGE_ROW + xNewCell].insert(object);

		object->Init();
	}
	break;

	case PACKET_TYPE::P_SC_GAMESTART_PACKET:
	{
		cout << "P_SC_GAMESTART_PACKET" << endl;
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CSelectScene* selectScene = reinterpret_cast<CSelectScene*>(scene);

		CObject* objects[3];
		string modelname[3] = { "Sonic","Mario","hugo_idle" };
		for (int i = 0; i < 3; ++i) {
			if (selectScene->m_selected_id[i] != -1) {
				objects[i] = CObject::Load(modelname[i]);
				objects[i]->m_id = selectScene->m_selected_id[i];
			}
		}

		CSceneManager::GetInstance()->ChangeScene(SCENE_TYPE::GAME);
		scene = CSceneManager::GetInstance()->GetCurrentScene();
		for (int i = 0; i < 3; ++i) {
			if (selectScene->m_selected_id[i] != -1) {
				objects[i]->SetGroupType((UINT)GROUP_TYPE::PLAYER);
				scene->m_objects[static_cast<int>(GROUP_TYPE::PLAYER)][objects[i]->m_id] = objects[i];
			}
		}
	}
	break;

	case PACKET_TYPE::P_SC_SELECT_PACKET:
	{
		SC_SELECT_PACKET* recv_packet = reinterpret_cast<SC_SELECT_PACKET*>(_Packet);

		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		reinterpret_cast<CSelectScene*>(scene)->m_character_names[recv_packet->m_char] += " : ";
		reinterpret_cast<CSelectScene*>(scene)->m_character_names[recv_packet->m_char] += recv_packet->m_name;
		reinterpret_cast<CSelectScene*>(scene)->m_selected_id[recv_packet->m_char] = recv_packet->m_id;
	}
	break;

	case PACKET_TYPE::P_SC_ADD_PACKET:	// NPC
	{
		SC_ADD_PACKET* recv_packet = reinterpret_cast<SC_ADD_PACKET*>(_Packet);
		cout << "SC_ADD_PLAYER" << endl;

		// ������Ʈ �ε�
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* object = CObject::Load(string(recv_packet->m_modelName));

		// ��ġ ����
		CTransform* transform = reinterpret_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
		transform->SetPosition(recv_packet->m_pos);
		transform->SetRotation(recv_packet->m_rota);
		transform->SetScale(recv_packet->m_sca);

		// ID ����
		object->m_id = recv_packet->m_id;

		object->Init();

		// ���� �߰�
		scene->AddObject((GROUP_TYPE)recv_packet->m_grouptype, object, recv_packet->m_id);
	}
	break;

	case PACKET_TYPE::P_SC_VELOCITY_CHANGE_PACKET:
	{
		SC_VELOCITY_CHANGE_PACKET* recv_packet = reinterpret_cast<SC_VELOCITY_CHANGE_PACKET*>(_Packet);
		cout << "SC_VELOCITY_CHANGE_PACKET" << endl;

		// �÷��̾� ��ü�� ���;� ��.
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		CObject* object = scene->GetIDObject((GROUP_TYPE)recv_packet->m_grouptype, recv_packet->m_id);

		if (object == nullptr) return;
		CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
		CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

		XMFLOAT3 vector = Vector3::TransformNormal(XMFLOAT3(0, 0, 1), Matrix4x4::Rotation(XMFLOAT3(0, recv_packet->m_angle, 0)));
		rigidBody->m_velocity = Vector3::ScalarProduct(vector, recv_packet->m_vel);
		CAnimator* animator = reinterpret_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		if (abs(recv_packet->m_vel - 15) < EPSILON) animator->SetSpeed(animator->m_upAnimation, 2);
		if(recv_packet->m_look != -1) 
			static_cast<CPlayer*>(object)->goal_rota = recv_packet->m_look;
		//static_cast<CPlayer*>(object)->goal_rota = recv_packet->m_angle;
		transform->SetPosition(recv_packet->m_pos);
	}
	break;

	case PACKET_TYPE::P_SC_POS_PACKET:
	{
		SC_POS_PACKET* recv_packet = reinterpret_cast<SC_POS_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetCurrentScene();
		if (scene->m_name == "GameScene")
		{
			CObject* obj = scene->GetIDObject((GROUP_TYPE)recv_packet->m_grouptype, recv_packet->m_id);
			if (obj != nullptr) {

				int xNewCell = clamp((int)(recv_packet->m_pos.x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
				int zNewCell = clamp((int)(recv_packet->m_pos.z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);

				CTransform* transform = static_cast<CTransform*>(obj->GetComponent(COMPONENT_TYPE::TRANSFORM));
				int oldX = clamp((int)(transform->GetPosition().x / (W_WIDTH / SECTOR_RANGE_COL)), 0, SECTOR_RANGE_COL - 1);
				int oldZ = clamp((int)(transform->GetPosition().z / (W_HEIGHT / SECTOR_RANGE_ROW)), 0, SECTOR_RANGE_ROW - 1);

				if (xNewCell != oldX || zNewCell != oldZ) {
					scene->ObjectListSector[oldZ * SECTOR_RANGE_ROW + oldX].erase(obj);
					scene->ObjectListSector[zNewCell * SECTOR_RANGE_ROW + xNewCell].insert(obj);
				}
				transform->SetPosition(recv_packet->m_pos);
				//transform->SetRotation(XMFLOAT3(0, recv_packet->m_rota, 0));
				//cout << "SC_POS_PACKET" << endl;
			}
		}
	}
	break;

	case PACKET_TYPE::P_SC_ANIMATION_PACKET:		// �ִϸ��̼�
	{
		SC_ANIMATION_PACKET* recv_packet = reinterpret_cast<SC_ANIMATION_PACKET*>(_Packet);

		CScene* scene = CSceneManager::GetInstance()->GetGameScene();
		CObject* object = scene->GetIDObject((GROUP_TYPE)recv_packet->m_grouptype, recv_packet->m_id);
		CAnimator* animator = reinterpret_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
		animator->Play(recv_packet->m_key, recv_packet->m_loop);
	}
	break;

	case PACKET_TYPE::P_SC_NPC_EXCHANGE_PACKET:		// NPC ��ȯ ���� ����
	{
		SC_NPC_EXCHANGE_PACKET* recv_packet = reinterpret_cast<SC_NPC_EXCHANGE_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetGameScene();
		switch (recv_packet->m_itemType) {
		case 0:
		{
			CObject* object = scene->GetIDObject(GROUP_TYPE::NPC, recv_packet->m_id);
			CNPC* npc = reinterpret_cast<CNPC*>(object);
			npc->m_needs.push_back(string(recv_packet->m_itemName));
			break;
		}
		case 1:
		{
			CObject* object = scene->GetIDObject(GROUP_TYPE::NPC, recv_packet->m_id);
			CNPC* npc = reinterpret_cast<CNPC*>(object);
			npc->m_outputs.push_back(string(recv_packet->m_itemName));
			break;
		}
		case 2:
		{
			CObject* object = scene->GetIDObject(GROUP_TYPE::BOX, recv_packet->m_id);
			CBox* box = reinterpret_cast<CBox*>(object);
			box->m_items.push_back(string(recv_packet->m_itemName));
			break;
		}
		case 3:
		{
			CObject* object = scene->GetIDObject(GROUP_TYPE::ONCE_ITEM, recv_packet->m_id);
			COnceItem* item = reinterpret_cast<COnceItem*>(object);
			item->m_items.push_back(string(recv_packet->m_itemName));
			break;
		}
		}
		break;
	}
	case PACKET_TYPE::P_SC_UPDATE_PHASE_PACKET:		// ������ ������Ʈ
	{
		SC_UPDATE_PHASE_PACKET* recv_packet = reinterpret_cast<SC_UPDATE_PHASE_PACKET*>(_Packet);
		
		std::chrono::duration<float> duration_in_float = recv_packet->m_time;
		float seconds_as_float = duration_in_float.count();

		CTimeManager::GetInstance()->m_phaseTime = seconds_as_float;
		CTimeManager::GetInstance()->m_lastTime = seconds_as_float;
		CTimeManager::GetInstance()->m_phase = recv_packet->m_phase;
		cout << "Phase " << recv_packet->m_phase << endl;
	}
	break;

	case PACKET_TYPE::P_SC_EXCHANGE_DONE_PACKET:	// NPC�� �ŷ� �Ϸ�
	{
		SC_EXCHANGE_DONE_PACKET* recv_packet = reinterpret_cast<SC_EXCHANGE_DONE_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetGameScene();
		CObject* object = scene->GetIDObject(GROUP_TYPE::NPC, recv_packet->m_npc_id);
		CNPC* npc = reinterpret_cast<CNPC*>(object);
		npc->m_standBy_id = recv_packet->m_npc_id;
	}
	break;

	case PACKET_TYPE::P_SC_PLAYER_RBUTTON_PACKET:	// ������ ��ư Ŭ��
	{
		SC_PLAYER_RBUTTON_PACKET* recv_packet = reinterpret_cast<SC_PLAYER_RBUTTON_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetGameScene();
		CObject* object = scene->GetIDObject(GROUP_TYPE::PLAYER, recv_packet->m_id);
		CPlayer* player = reinterpret_cast<CPlayer*>(object);
		player->goal_rota = recv_packet->m_angle;
	}
	break;

	case PACKET_TYPE::P_SC_HEALTH_CHANGE_PACKET:
	{
		SC_HEALTH_CHANGE_PACKET* recv_packet = reinterpret_cast<SC_HEALTH_CHANGE_PACKET*>(_Packet);
		CScene* scene = CSceneManager::GetInstance()->GetGameScene();
		CObject* object = scene->GetIDObject((GROUP_TYPE)recv_packet->m_groupType, recv_packet->m_id);

		CCharacter* character = reinterpret_cast<CCharacter*>(object);
		character->SetHealth(recv_packet->m_health);
		cout << recv_packet->m_id << " - " << character->GetHealth() << endl;
	}
	break;

	default:
		break;
	}
}