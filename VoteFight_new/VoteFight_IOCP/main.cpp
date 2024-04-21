#pragma once
#include "pch.h"
#include "./ImaysNet/ImaysNet.h"

using namespace std;

volatile bool				stopServer = false;
const int					numWorkerTHREAD{ 1 };	// Worker Thread Count
static unsigned long long	nextClientID{ 0 };		// Next Client ID

vector<shared_ptr<thread>>	workerThreads;			// Worker Thread Vector
recursive_mutex				mx;						// ����� Mutex

enum class IO_TYPE;									// I/O Type

class RemoteClient {
public:
	shared_ptr<thread>		m_thread;			// Client thread
	Socket					m_tcpConnection;	// Acceept Tcp
	unsigned long long		m_id;				// Client ID

public:
	RemoteClient() : m_thread(), m_tcpConnection(SocketType::Tcp) {}
	RemoteClient(SocketType _socketType) : m_tcpConnection(_socketType) {}
};

// Ŭ���̾�Ʈ �����Ϳ� Ŭ���̾�Ʈ�� ���� �����͸� �����ϴ� �ؽ� ��
unordered_map<RemoteClient*, shared_ptr<RemoteClient>>	remoteClients;

// Ŭ���̾�Ʈ���� ����Ű�� �����͸� �����ϴ� ����
vector<RemoteClient*>									remoteClients_ptr_v;

// ���� Ŭ���̾�Ʈ ���
list<shared_ptr<RemoteClient>>							deleteClinets;

Iocp						iocp(numWorkerTHREAD);	// IOCP Init
recursive_mutex				mx_accept;				
shared_ptr<Socket>			listenSocket;			
shared_ptr<RemoteClient>	remoteClientCandidate; 

// Client ���� ó�� �Լ�
void	ProcessClientLeave(shared_ptr<RemoteClient> _remoteClient);
void	WorkerThread();
void	ProcessAccept();
void	PacketProcess(shared_ptr<RemoteClient>& _Client, char* _Packet);
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
		lock_guard<recursive_mutex> lock_rc(mx);
		remoteClients.erase(_remoteClient.get());	// Ŭ���̾�Ʈ ����

		cout << " >> Client left. There are " << remoteClients.size() << " connections.\n";
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
					cout << " >> Send - type : " << (int)p_readOverlapped->m_buf[1] << endl;
					p_readOverlapped->m_isReadOverlapped = false;
					continue;
				}

				if (readEvent.lpCompletionKey == 0) {    // Listetn socket
					ProcessAccept(); // Client Accept
				}
				else { // TCP Socket
					shared_ptr<RemoteClient> remoteClient;	// ó���� Ŭ���̾�Ʈ ��������
					{
						lock_guard<recursive_mutex> lock_rc(mx);
						remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
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
		remoteClient->m_id = nextClientID++;
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
				lock_guard<recursive_mutex> lock_rc(mx);
				remoteClients.insert({ remoteClient.get(), remoteClient });

				cout << " >> Client joined. There are " << remoteClients.size() << " connections.\n";
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
	// i/o �Ϸ� üũ & ���� �ݱ�
	listenSocket->Close();
	{
		lock_guard<recursive_mutex> lock_rc(mx);

		for (auto i : remoteClients)
			i.second->m_tcpConnection.Close();

		// ���� ��
		cout << " >> Shutting down the server....\n";
		while (remoteClients.size() > 0) {
			// I/O completion�� ���� ������ RemoteClient�� ����
			for (auto i = remoteClients.begin(); i != remoteClients.end(); ++i) {
				if (!i->second->m_tcpConnection.m_isReadOverlapped)
					remoteClients.erase(i);
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
					shared_ptr<RemoteClient> remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient) {
						remoteClient->m_tcpConnection.m_isReadOverlapped = false;
					}
				}
			}
		}
	}
	std::cout << " >> Server End.\n";
}