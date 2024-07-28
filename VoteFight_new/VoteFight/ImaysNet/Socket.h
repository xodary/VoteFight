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
// SOCKET은 64bit 환경에서 64bit이다. 반면 linux에서는 여전히 32bit이다. 이 차이를 위함.
typedef int SOCKET;
#endif

#define MAX_SOCKBUF 1024 // 패킷(현재는 버퍼)크기
#define MAX_CLIENT 100 // 최대 접속가능한 클라이언트 수
#define MAX_WORKERTHREAD 4 // 쓰레드 풀(CP객체)에 넣을 쓰레드 수

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
	WSAOVERLAPPED	m_wsa_over;						// WSAOVERLAPPED 구조체 - 비동기 소켓 작업 관리
	IO_TYPE			m_ioType;						// 소켓 작업의 종류
	WSABUF			m_wsa_buf;						// 소켓 입출력 작업에 사용되는 데이터 저장 버퍼
	char			m_buf[MAX_SOCKBUF];				// 실제 데이터 저장 버퍼
	char			m_isReadOverlapped = false;		// 소켓 작업의 오버랩 여부 플래그

public:
	EXP_OVER() : m_ioType(IO_TYPE::IO_RECV) {
		ZeroMemory(&m_wsa_over, sizeof(m_wsa_over));
		m_wsa_buf.buf = m_buf;
		m_wsa_buf.len = MAX_SOCKBUF;
		ZeroMemory(&m_buf, sizeof(m_buf));
	}
	// 패킷 데이터를 사용하여 송신용 오버랩 구조체를 초기화
	EXP_OVER(const char* _packet) : m_ioType(IO_TYPE::IO_SEND)
	{
		ZeroMemory(&m_wsa_over, sizeof(m_wsa_over));
		m_wsa_buf.buf = m_buf;
		m_wsa_buf.len = _packet[0];
		ZeroMemory(&m_buf, sizeof(m_buf));
		memcpy(m_buf, _packet, _packet[0]);
	}
	// 버퍼와 크기를 사용하여 송신용 오버랩 구조체를 초기화
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

// 소켓 클래스
class Socket
{
public:
	SOCKET			m_fd; // 소켓 핸들

#ifdef _WIN32
	// AcceptEx 함수 포인터
	LPFN_ACCEPTEX	AcceptEx = NULL;

	// Overlapped I/O나 IOCP를 쓸 때에만 사용됩니다. 현재 overlapped I/O 중이면 true입니다.
	// (N-send나 N-recv도 가능하게 하려면 이렇게 단일 값만 저장하면 안되지만, 본 소스는 독자의 학습이 우선이므로 이를 고려하지 않았습니다.)
	bool m_isReadOverlapped = false;

	// Overlapped receive or accept을 할 때 사용되는 overlapped 객체입니다. 
	// I/O 완료 전까지는 보존되어야 합니다.
//	WSAOVERLAPPED m_readOverlappedStruct;
#endif
	// Receive나 ReceiveOverlapped에 의해 수신되는 데이터가 채워지는 곳입니다.
	// overlapped receive를 하는 동안 여기가 사용됩니다. overlapped I/O가 진행되는 동안 이 값을 건드리지 마세요.
//	char m_receiveBuffer[MaxReceiveLength];

	EXP_OVER								m_recvOverlapped = EXP_OVER(); // Recv Overlapped(비동기) I/O 작업을 위한 변수
	std::list<std::shared_ptr<EXP_OVER>>	m_sendOverlapped_list;
	unsigned char							m_prev_remain = 0;
#ifdef _WIN32
	// overlapped 수신을 하는 동안 여기에 recv의 flags에 준하는 값이 채워집니다. overlapped I/O가 진행되는 동안 이 값을 건드리지 마세요.
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
