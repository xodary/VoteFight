#pragma once
#include "pch.h"
#include "./ImaysNet/ImaysNet.h"
#include "RemoteClient.h"

volatile bool				stopServer = false;
const int					numWorkerTHREAD{ 1 };	// Worker Thread Count
static unsigned long long	nextClientID{ 0 };		// Next Client ID

vector<shared_ptr<thread>>	workerThreads;			// Worker Thread Vector
enum class IO_TYPE;									// I/O Type

// 클라이언트들을 가리키는 포인터를 저장하는 벡터
vector<RemoteClient*>		remoteClients_ptr_v;

// 삭제 클라이언트 목록
list<shared_ptr<RemoteClient>>		deleteClinets;

Iocp						iocp(numWorkerTHREAD);	// IOCP Init
recursive_mutex				mx_accept;				
shared_ptr<Socket>			listenSocket;			
shared_ptr<RemoteClient>	remoteClientCandidate; 

// Client 종료 처리 함수
void	ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient);
void	ProcessAccept();
void	PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet);
void	WorkerThread();
void	CloseServer();

// 서버 프로그램 진입점
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
			iocp.Wait(readEvents, 100);

			// 받은 이벤트 처리
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
					shared_ptr<RemoteClient> remoteClient;	// 처리할 클라이언트 가져오기
					{
						lock_guard<recursive_mutex> lock_rc(RemoteClient::m_lock);
						remoteClient = RemoteClient::m_remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					}

					//
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

							cout << " >> Recv - recv_buf_Length : " << recv_buf_Length << endl;

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
		remoteClient->m_id = nextClientID++;
		remoteClients_ptr_v.emplace_back(remoteClient.get());

		// 소켓 IOCP에 추가
		iocp.Add(remoteClient->m_tcpConnection, remoteClient.get());

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

		{	// Send connected client info
			SC_INIT_PACKET send_packet;
			send_packet.m_size = sizeof(SC_INIT_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_INIT_PACKET;
			send_packet.m_id = _Client->m_id;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		 
		// Send all player infomation to connected Client (새로 연결된 클라이언트가 다른 클라이언트들의 정보를 알 수 있도록 함)
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (rc.second->m_id == _Client->m_id)
				continue;
			SC_ADD_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ADD_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_ADD_PACKET;
			send_packet.m_id = rc.second->m_id;
			_Client->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}

		// Send connected client infomation to other client (다른 모든 클라이언트에게 특정 클라이언트의 정보를 보냄)
		for (auto& rc : RemoteClient::m_remoteClients) {
			if (rc.second->m_id == _Client->m_id)
				continue;
			SC_ADD_PACKET send_packet;
			send_packet.m_size = sizeof(SC_ADD_PACKET);
			send_packet.m_type = PACKET_TYPE::P_SC_ADD_PACKET;
			send_packet.m_id = _Client->m_id;
			rc.second->m_tcpConnection.SendOverlapped(reinterpret_cast<char*>(&send_packet));
		}
		break;
	}
	case PACKET_TYPE::P_CS_MOVE_PACKET:
	{
		break;
	}
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
			iocp.Wait(readEvents, 100);

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