#pragma once
#include "pch.h"
#include "./ImaysNet/ImaysNet.h"
#include "RemoteClient.h"

volatile bool				stopServer = false;
const int					numWorkerTHREAD{ 1 };	// Worker Thread Count
static unsigned long long	nextClientID{ 0 };		// Next Client ID

vector<shared_ptr<thread>>	workerThreads;			// Worker Thread Vector
enum class IO_TYPE;									// I/O Type

// Ŭ���̾�Ʈ���� ����Ű�� �����͸� �����ϴ� ����
vector<RemoteClient*>		remoteClients_ptr_v;

// ���� Ŭ���̾�Ʈ ���
list<shared_ptr<RemoteClient>>		deleteClinets;

Iocp						iocp(numWorkerTHREAD);	// IOCP Init
recursive_mutex				mx_accept;				
shared_ptr<Socket>			listenSocket;			
shared_ptr<RemoteClient>	remoteClientCandidate; 

// Client ���� ó�� �Լ�
void	ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient);
void	ProcessAccept();
void	PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet);
void	WorkerThread();
void	CloseServer();

// ���� ���α׷� ������
int main(int argc, char* argv[])
{
	// Create listen socket & Binding
	listenSocket = make_shared<Socket>(SocketType::Tcp);
	listenSocket->Bind(Endpoint("0.0.0.0", SERVER_PORT));
	listenSocket->Listen();

	// IOCP Init & Add
	iocp.Add(*listenSocket,listenSocket.get());

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
	for (int i = 0 ; i < numWorkerTHREAD; ++i)
		workerThreads.emplace_back(make_shared<thread>(WorkerThread));

	for (auto& th : workerThreads) 
		th->join();

	// Sever End
	CloseServer();
}

void ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient)
{
	// ���� Ȥ�� ���� ���� ��
	_remoteClient->m_tcpConnection.Close();
	{
		lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
		RemoteClient::m_remoteClients.erase(_remoteClient.get());	// Ŭ���̾�Ʈ ����

		cout << " >> Client left. There are " << RemoteClient::m_remoteClients.size() << " connections.\n";
	}
}
void WorkerThread()
{
	try {
		while (!stopServer) {
			// I/O �Ϸ� �̺�Ʈ ���
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// ���� �̺�Ʈ ó��
			for (int i = 0; i < readEvents.m_eventCount; ++i) {
				auto& readEvent = readEvents.m_events[i];
				auto p_readOverlapped = (EXP_OVER*)readEvent.lpOverlapped;

				if (IO_TYPE::IO_SEND == p_readOverlapped->m_ioType) {
					cout << " >> Send - size : " << (int)p_readOverlapped->m_buf[0] << endl;
					cout << " >> Send - type : " << (int)p_readOverlapped->m_buf[1] << endl;
					p_readOverlapped->m_isReadOverlapped = false;
					continue;
				}

				if (readEvent.lpCompletionKey == (ULONG_PTR)listenSocket.get()) {    // Listetn socket
					ProcessAccept(); // Client Accept
				}
				else { // TCP Socket
					shared_ptr<RemoteClient> remoteClient;	// ó���� Ŭ���̾�Ʈ ��������
					{
						lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
						remoteClient = RemoteClient::m_remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					}

					//
					if (remoteClient) { // ���� �Ϸ� ����. �Ϸ�� �� ������ �۾�
						remoteClient->m_tcpConnection.m_isReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0) { //  0 - TCP ���� ����, -1 : Error
							ProcessClientLeave(remoteClient);
						}
						else {
							// ���� �Ϸ� ����. �Ϸ�� �� ������ �۾�
							char* recv_buf = remoteClient->m_tcpConnection.m_recvOverlapped.m_buf;
							int recv_buf_Length = ec;

							cout << " >> Recv - recv_buf_Length : " << recv_buf_Length << endl;

							{	// ��Ŷ ó��
								int remain_data = recv_buf_Length + remoteClient->m_tcpConnection.m_prev_remain;
								while (remain_data > 0) {
									unsigned char packet_size = recv_buf[0];
									if (packet_size > remain_data) // // ���� �����Ͱ� ���� ó���� ��Ŷ ũ�⺸�� ������ �߷Ȱų� �� ������.
										break;

									//��Ŷ ó��
									PacketProcess(remoteClient, recv_buf);

									//���� ��Ŷ �̵�, ���� ������ ����
									recv_buf += packet_size;
									remain_data -= packet_size;

								}
								//���� ������ ����
								remoteClient->m_tcpConnection.m_prev_remain = remain_data;

								//���� �����Ͱ� 0�� �ƴ� ���� ������ recv_buf�� �� ������ ����
								if (remain_data > 0) {
									memcpy(remoteClient->m_tcpConnection.m_recvOverlapped.m_buf, recv_buf, remain_data);
								}
							}

							// ���� ���� �غ�
							if (remoteClient->m_tcpConnection.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING) {
								ProcessClientLeave(remoteClient);
							}
							else {
								// I/O�� �ɰ� �ϷḦ ��� ���·� ����
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
	// Accept�� �̹� �Ϸ� -> ������ �۾�
	if (remoteClientCandidate->m_tcpConnection.UpdateAcceptContext(*listenSocket) != 0) {
		listenSocket->Close();
	}
	else {
		shared_ptr<RemoteClient> remoteClient = remoteClientCandidate;
		remoteClients_ptr_v.emplace_back(remoteClient.get());

		// ���� IOCP�� �߰�
		iocp.Add(remoteClient->m_tcpConnection, remoteClient.get());

		// overlapped ���� ��û
		if (remoteClient->m_tcpConnection.ReceiveOverlapped() != 0
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			remoteClient->m_tcpConnection.Close();
		}
		else {
			// I/O�� �ɰ� �ϷḦ ��� ���·� ����
			remoteClient->m_tcpConnection.m_isReadOverlapped = true;
			{
				lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
				RemoteClient::m_remoteClients.insert({ remoteClient.get(), remoteClient });

				cout << " >> Client joined. There are " << RemoteClient::m_remoteClients.size() << " connections.\n";
			}
		}

		// ����ؼ� ���� �޾ƾ� �ϹǷ� Listen ���Ͼ� overlapped I/O�� �ɾ��
		remoteClientCandidate = make_shared<RemoteClient>(SocketType::Tcp);
		string errorText;
		if (!listenSocket->AcceptOverlapped(remoteClientCandidate->m_tcpConnection, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING) {
			listenSocket->Close();
		}
		else {
			// Listen ������ ������ ��ٸ��� ����
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

		SC_LOGIN_OK_PACKET send_packet;
		send_packet.m_size = sizeof(SC_LOGIN_OK_PACKET);
		send_packet.m_type = PACKET_TYPE::P_SC_LOGIN_OK_PACKET;
		send_packet.m_id = _Client->m_id;
		send_packet.m_xPos = _Client->getXpos() * (_Client->m_id);
		send_packet.m_yPos = _Client->getYpos();
		send_packet.m_zPos = _Client->getZpos();
		_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		cout << " >> Send ) LOGIN_OK_PACKET Client ID : " << _Client->m_id << endl;
		// cout << "SC_LOGIN_OK_PACKET - X : " << p.getXpos() * (_Client->m_id) << ", Y : " << p.getYpos() << ", Z : " << p.getZpos() << endl;

		// Send all player infomation to connected Client (���� ����� Ŭ���̾�Ʈ�� �ٸ� Ŭ���̾�Ʈ���� ������ �� �� �ֵ��� ��)
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (rc.second->m_id == _Client->m_id)
				continue;
			SC_ADD_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ADD_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_ADD_PACKET;
			send_packet.m_id = rc.second->m_id;
			send_packet.m_xPos = _Client->getXpos() * (_Client->m_id);
			send_packet.m_yPos = _Client->getYpos();
			send_packet.m_zPos = _Client->getZpos();
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> Send ) Add Packet 1 Client ID : " << _Client->m_id<< endl;
			cout << " >> send ) Send Add packet 1" << endl;
		}

		// Send connected client infomation to other client (�ٸ� ��� Ŭ���̾�Ʈ���� Ư�� Ŭ���̾�Ʈ�� ������ ����)
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (rc.second->m_id == _Client->m_id)
				continue;
			SC_ADD_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ADD_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_ADD_PACKET;
			send_packet.m_id = _Client->m_id;
			send_packet.m_xPos = _Client->getXpos() * (_Client->m_id);
			send_packet.m_yPos = _Client->getYpos();
			send_packet.m_zPos = _Client->getZpos();
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			cout << " >> Send ) Add Packet 2 Client ID : " << _Client->m_id << endl;
			cout << " >> send ) Send Add packet 2" << endl;
		}
		break;
	}

	case PACKET_TYPE::P_CS_WALK_ENTER_PACKET:
	{
		// cout << " >> recv ) CS_WALK_ENTER_PACKET" << endl;
		CS_WALK_ENTER_PACEKET* recv_packet = reinterpret_cast<CS_WALK_ENTER_PACEKET*>(_Packet);
		{
			SC_WALK_ENTER_INFO_PACKET send_packet;
			send_packet.m_size = sizeof(SC_WALK_ENTER_INFO_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_WALK_ENTER_INFO_PACKET;
			send_packet.m_key = "lisaWalk";
			send_packet.m_maxSpeed = 400.f;
			send_packet.m_vel = 400.f;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			// cout << " >> send ) SC_WALK_ENTER_INFO_PACKET" << endl;
		}
		break;
	}

	case PACKET_TYPE::P_CS_MOVE_PACKET:
	{
		CS_MOVE_PACKET* recv_packet = reinterpret_cast<CS_MOVE_PACKET*>(_Packet);

		break;
	}

	case PACKET_TYPE::P_CS_MOVE_V_PACKET:
	{
		// cout << " >> P_CS_MOVE_V_PACKET" << endl;
		CS_MOVE_V_PACKET* recv_packet = reinterpret_cast<CS_MOVE_V_PACKET*>(_Packet);
		// cout << " [01] Recv ID : " << recv_packet->m_id << endl;

		for (auto& rc : RemoteClient::m_remoteClients) {
			if (_Client->m_id == rc.second->m_id) {
				RemoteClient::m_lock.lock();
				rc.second->m_player->setXpos(recv_packet->m_vec.x);
				rc.second->m_player->setYpos(recv_packet->m_vec.y);
				rc.second->m_player->setZpos(recv_packet->m_vec.z);
				// cout << " [02] _Client->m_id : " << _Client->m_id << endl;
				// cout << " [03] rc.second->m_id : " << rc.second->m_id << endl;
				// cout << " [04] Recv Pos : " << recv_packet->m_vec.x << ", "<< recv_packet->m_vec.y <<", " << recv_packet->m_vec.z <<  endl;
				RemoteClient::m_lock.unlock();
				continue;
			}

			SC_MOVE_V_PACKET send_packet;
			send_packet.m_size = sizeof(SC_MOVE_V_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_MOVE_V_PACKET;
			send_packet.m_id = rc.second->m_id;
			send_packet.m_vec = recv_packet->m_vec;
			send_packet.m_rota = recv_packet->m_rota;
			send_packet.m_state = recv_packet->m_state;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
			// cout << " [05] send ID : " << rc.second->m_id << endl;
			// std::cout << "rc ID - " << rc.second->m_id << ", xPos - " << recv_packet->m_vec.x << ", yPos - " << recv_packet->m_vec.y << ", zPos - " << recv_packet->m_vec.z << endl;
		}

		break;
	}

	default:
		break;
	}
}
void CloseServer()
{
	lock_guard<recursive_mutex> lock_accept(mx_accept);
	// i/o �Ϸ� üũ & ���� �ݱ�
	listenSocket->Close();
	{
		lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);

		for (auto i : RemoteClient::m_remoteClients)
			i.second->m_tcpConnection.Close();

		// ���� ��
		cout << " >> Shutting down the server....\n";
		while (RemoteClient::m_remoteClients.size() > 0) {
			// I/O completion�� ���� ������ RemoteClient�� ����
			for (auto i = RemoteClient::m_remoteClients.begin(); i != RemoteClient::m_remoteClients.end(); ++i) {
				if (!i->second->m_tcpConnection.m_isReadOverlapped)
					RemoteClient::m_remoteClients.erase(i);
			}

			// I/O completion�� �߻� �� �� �̻� Overlapped I/O�� ���� ���� ���� ��ȣ Flag.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// ���� �̺�Ʈ ������ ó���մϴ�.
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