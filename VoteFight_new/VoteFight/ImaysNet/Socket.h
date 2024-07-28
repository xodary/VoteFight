#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <memory>
#include <list>
#else 
#include <sys/socket.h>
#endif

#ifndef _WIN32
// SOCKET�� 64bit ȯ�濡�� 64bit�̴�. �ݸ� linux������ ������ 32bit�̴�. �� ���̸� ����.
typedef int SOCKET;
#endif

#define MAX_SOCKBUF 1024 // ��Ŷ(����� ����)ũ��
#define MAX_CLIENT 100 // �ִ� ���Ӱ����� Ŭ���̾�Ʈ ��
#define MAX_WORKERTHREAD 4 // ������ Ǯ(CP��ü)�� ���� ������ ��

class Endpoint;

enum class SocketType {
	Tcp,
	Udp,
};

enum class IO_TYPE {
	IO_RECV,
	IO_SEND,
};

class EXP_OVER {
public:
	WSAOVERLAPPED	m_wsa_over;						// WSAOVERLAPPED ����ü - �񵿱� ���� �۾� ����
	IO_TYPE			m_ioType;						// ���� �۾��� ����
	WSABUF			m_wsa_buf;						// ���� ����� �۾��� ���Ǵ� ������ ���� ����
	char			m_buf[MAX_SOCKBUF];				// ���� ������ ���� ����
	char			m_isReadOverlapped = false;		// ���� �۾��� ������ ���� �÷���

public:
	EXP_OVER() : m_ioType(IO_TYPE::IO_RECV) {
		ZeroMemory(&m_wsa_over, sizeof(m_wsa_over));
		m_wsa_buf.buf = m_buf;
		m_wsa_buf.len = MAX_SOCKBUF;
		ZeroMemory(&m_buf, sizeof(m_buf));
	}
	// ��Ŷ �����͸� ����Ͽ� �۽ſ� ������ ����ü�� �ʱ�ȭ
	EXP_OVER(const char* _packet) : m_ioType(IO_TYPE::IO_SEND)
	{
		ZeroMemory(&m_wsa_over, sizeof(m_wsa_over));
		m_wsa_buf.buf = m_buf;
		m_wsa_buf.len = _packet[0];
		ZeroMemory(&m_buf, sizeof(m_buf));
		memcpy(m_buf, _packet, _packet[0]);
	}
	// ���ۿ� ũ�⸦ ����Ͽ� �۽ſ� ������ ����ü�� �ʱ�ȭ
	EXP_OVER(const char* _buf, short _buf_size) : m_ioType(IO_TYPE::IO_SEND)
	{
		ZeroMemory(&m_wsa_over, sizeof(m_wsa_over));
		m_wsa_buf.buf = m_buf;
		m_wsa_buf.len = _buf_size;
		ZeroMemory(&m_buf, sizeof(m_buf));
		memcpy(m_buf, _buf, _buf_size);
	}

	~EXP_OVER() {}
};

// ���� Ŭ����
class Socket
{
public:
	SOCKET			m_fd; // ���� �ڵ�

#ifdef _WIN32
	// AcceptEx �Լ� ������
	LPFN_ACCEPTEX	AcceptEx = NULL;

	// Overlapped I/O�� IOCP�� �� ������ ���˴ϴ�. ���� overlapped I/O ���̸� true�Դϴ�.
	// (N-send�� N-recv�� �����ϰ� �Ϸ��� �̷��� ���� ���� �����ϸ� �ȵ�����, �� �ҽ��� ������ �н��� �켱�̹Ƿ� �̸� ������� �ʾҽ��ϴ�.)
	bool m_isReadOverlapped = false;

	// Overlapped receive or accept�� �� �� ���Ǵ� overlapped ��ü�Դϴ�. 
	// I/O �Ϸ� �������� �����Ǿ�� �մϴ�.
//	WSAOVERLAPPED m_readOverlappedStruct;
#endif
	// Receive�� ReceiveOverlapped�� ���� ���ŵǴ� �����Ͱ� ä������ ���Դϴ�.
	// overlapped receive�� �ϴ� ���� ���Ⱑ ���˴ϴ�. overlapped I/O�� ����Ǵ� ���� �� ���� �ǵ帮�� ������.
//	char m_receiveBuffer[MaxReceiveLength];

	EXP_OVER								m_recvOverlapped = EXP_OVER(); // Recv Overlapped(�񵿱�) I/O �۾��� ���� ����
	std::list<std::shared_ptr<EXP_OVER>>	m_sendOverlapped_list;
	unsigned char							m_prev_remain = 0;
#ifdef _WIN32
	// overlapped ������ �ϴ� ���� ���⿡ recv�� flags�� ���ϴ� ���� ä�����ϴ�. overlapped I/O�� ����Ǵ� ���� �� ���� �ǵ帮�� ������.
	DWORD									m_readFlags = 0;
#endif

	Socket();
	Socket(SOCKET fd);
	Socket(SocketType socketType);
	~Socket();

	void	Bind(const Endpoint& endpoint);
	void	Connect(const Endpoint& endpoint);
	int		Send(const char* data, int length);
	void	Close();
	void	Listen();
	int		Accept(Socket& acceptedSocket, std::string& errorText);
#ifdef _WIN32
	bool	AcceptOverlapped(Socket& acceptCandidateSocket, std::string& errorText);
	int		UpdateAcceptContext(Socket& listenSocket);
#endif
	Endpoint GetPeerAddr();
	int		Receive();
#ifdef _WIN32
	int		ReceiveOverlapped();
	int		SendOverlapped(const char* packet);
#endif
	void	SetNonblocking();
	
	void Disconnect();
};

std::string	 GetLastErrorAsString();

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif
