#include "../pch.h"
#ifdef _WIN32
#include <rpc.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include <algorithm>
#include <sstream>
#include "../GameFramework.h"
#include "../SceneManager.h"
#include "../LoginScene.h"
#include "../Scene.h"
#include "Socket.h"
#include "Endpoint.h"
#include "SocketInit.h"
#include "Exception.h"

std::string GetLastErrorAsString();

// 소켓을 생성하는 생성자.
Socket::Socket(SocketType socketType)
{
	g_socketInit.Touch();

	// overlapped I/O를 쓰려면 socket() 말고 WSASocket을 써야 합니다.
	if(socketType==SocketType::Tcp)
	{
#ifdef _WIN32
		m_fd = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
		m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
	}
	else 
	{
#ifdef _WIN32
		m_fd = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
#else
		m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
	}

#ifdef _WIN32
	ZeroMemory(&m_recvOverlapped, sizeof(m_recvOverlapped));
#endif
}

// 외부 소켓 핸들을 받는 생성자.
Socket::Socket(SOCKET fd)
{
	g_socketInit.Touch();
	m_fd = fd;

#ifdef _WIN32
	ZeroMemory(&m_recvOverlapped, sizeof(m_recvOverlapped));
#endif
}

// 소켓을 생성하지는 않는다.
Socket::Socket()
{
#ifdef _WIN32
	static_assert(-1 == INVALID_SOCKET, "");
#endif

	m_fd = -1;

#ifdef _WIN32
	ZeroMemory(&m_recvOverlapped, sizeof(m_recvOverlapped));
#endif
}

Socket::~Socket()
{
	Close();
}

void Socket::Bind(const Endpoint& endpoint)
{
	if (::bind(m_fd, (sockaddr*)&endpoint.m_ipv4Endpoint, sizeof(endpoint.m_ipv4Endpoint)) < 0)
	{
		stringstream ss;
		ss << "bind failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
}

// endpoint가 가리키는 주소로의 접속을 합니다.
bool Socket::Connect(const Endpoint& endpoint)
{
	try {
		if (connect(m_fd, (sockaddr*)&endpoint.m_ipv4Endpoint, sizeof(endpoint.m_ipv4Endpoint)) < 0)
		{
			stringstream ss;
			ss << "connect failed:" << GetLastErrorAsString();
			throw Exception(ss.str().c_str());
		}
	}
	catch(Exception ex){
		CGameFramework::GetInstance()->m_connect_server = false;
		CLoginScene* loginscene = reinterpret_cast<CLoginScene*>(CSceneManager::GetInstance()->GetScene(SCENE_TYPE::LOGIN));
		strcpy_s(loginscene->login_state, ex.m_text.c_str());
		return false;
	}
	return true;
}

// 송신을 합니다.
int Socket::Send(const char* data, int length)
{
	return ::send(m_fd, data, length, 0);
}

void Socket::Close()
{
#ifdef _WIN32
	closesocket(m_fd);
#else
	close(m_fd);
#endif
}

void Socket::Listen()
{
	listen(m_fd, 5000);
}

// 성공하면 0, 실패하면 다른 값을 리턴합니다.
// errorText에는 실패시 에러내용이 텍스트로  채워집니다.
// acceptedSocket에는 accept된 소켓 핸들이 들어갑니다.
int Socket::Accept(Socket& acceptedSocket, string& errorText)
{
	acceptedSocket.m_fd = accept(m_fd, NULL, 0);
	if (acceptedSocket.m_fd == -1)
	{
		errorText = GetLastErrorAsString();
		return -1;
	}
	else
		return 0;
}

#ifdef _WIN32

// 성공하면 true, 실패하면 false를 리턴합니다.
// errorText에는 실패시 에러내용이 텍스트로  채워집니다.
// acceptCandidateSocket에는 이미 만들어진 소켓 핸들이 들어가며, accept이 되고 나면 이 소켓 핸들은 TCP 연결 객체로 변신합니다.
bool Socket::AcceptOverlapped(Socket& acceptCandidateSocket, string& errorText)
{
	if (AcceptEx == NULL)
	{
		DWORD bytes;
		// AcceptEx는 여타 소켓함수와 달리 직접 호출하는 것이 아니고,
		// 함수 포인터를 먼저 가져온 다음 호출할 수 있다. 그것을 여기서 한다.
		UUID uuid{ UUID(WSAID_ACCEPTEX) };

		WSAIoctl(m_fd,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(UUID),
			&AcceptEx,
			sizeof(AcceptEx),
			&bytes,
			NULL,
			NULL);

		if (AcceptEx == NULL)
		{
			throw Exception("Getting AcceptEx ptr failed.");
		}
	}


	// 여기에는 accept된 소켓의 로컬주소와 리모트주소가 채워집니다만 본 예제에서 독자들에게 가르쳐줄 범위를 벗어나므로 그냥 버립니다.
	char ignored[200];
	DWORD ignored2 = 0;

	bool ret = AcceptEx(m_fd,
		acceptCandidateSocket.m_fd,
		&ignored,
		0,
		50,
		50,
		&ignored2,
		(WSAOVERLAPPED*)&m_recvOverlapped
	) == TRUE;
	
	return ret;
}


// AcceptEx가 I/O 완료를 하더라도 아직 TCP 연결 받기 처리가 다 끝난 것이 아니다.
// 이 함수를 호출해주어야만 완료가 된다.
int Socket::UpdateAcceptContext(Socket& listenSocket)
{
	sockaddr_in ignore1;
	sockaddr_in ignore3;
	INT ignore2,ignore4;

	char ignore[1000];
	GetAcceptExSockaddrs(ignore,
		0,
		50,
		50,
		(sockaddr**)&ignore1,
		&ignore2,
		(sockaddr**)&ignore3,
		&ignore4);

	return setsockopt(m_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSocket.m_fd, sizeof(listenSocket.m_fd));
}

#endif // _WIN32

Endpoint Socket::GetPeerAddr()
{
	Endpoint ret;
	socklen_t retLength = sizeof(ret.m_ipv4Endpoint);
	if (::getpeername(m_fd, (sockaddr*)&ret.m_ipv4Endpoint, &retLength) < 0)
	{
		stringstream ss;
		ss << "getPeerAddr failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
	if(retLength > sizeof(ret.m_ipv4Endpoint))
	{
		stringstream ss;
		ss << "getPeerAddr buffer overrun: " << retLength;
		throw Exception(ss.str().c_str());
	}

	return ret;
}

// 소켓 수신을 합니다. 
// 블로킹 소켓이면 1바이트라도 수신하거나 소켓 에러가 나거나 소켓 연결이 끊어질 때까지 기다립니다.
// 논블로킹 소켓이면 기다려야 하는 경우 즉시 리턴하고 EWOULDBLOCK이 errno나 GetLastError에서 나오게 됩니다.
// 리턴값: recv 리턴값 그대로입니다.
int Socket::Receive()
{
	return (int)recv(m_fd, m_recvOverlapped.m_buf, MAX_SOCKBUF, 0);
}

#ifdef _WIN32

// overlapeed 수신을 겁니다. 즉 백그라운드로 수신 처리를 합니다.
// 수신되는 데이터는 m_receiveBuffer에 비동기로 채워집니다.
// 리턴값: WSARecv의 리턴값 그대로입니다.
int Socket::ReceiveOverlapped()
{
	// overlapped I/O가 진행되는 동안 여기 값이 채워집니다.
	m_readFlags = 0;
	ZeroMemory(&m_recvOverlapped.m_wsa_over, sizeof(m_recvOverlapped.m_wsa_over));
	m_recvOverlapped.m_wsa_buf.len = MAX_SOCKBUF - m_prev_remain;
	m_recvOverlapped.m_wsa_buf.buf = m_recvOverlapped.m_buf + m_prev_remain;

	return WSARecv(m_fd, &m_recvOverlapped.m_wsa_buf, 1, NULL, &m_readFlags, (WSAOVERLAPPED*)&m_recvOverlapped, NULL);
}

int Socket::SendOverlapped(const char* packet)
{
	auto p = find_if(m_sendOverlapped_list.begin(), m_sendOverlapped_list.end(), [](shared_ptr<EXP_OVER>& lhs) {
		return lhs->m_isReadOverlapped == false;
		});

	if (p != m_sendOverlapped_list.end()) {
		ZeroMemory(&(*p)->m_wsa_over, sizeof((*p)->m_wsa_over));
		(*p)->m_wsa_buf.len = packet[0];
		ZeroMemory(&(*p)->m_buf, sizeof((*p)->m_buf));
		memcpy((*p)->m_buf, packet, packet[0]);
		(*p)->m_isReadOverlapped = true;
		return WSASend(m_fd, &(*p)->m_wsa_buf, 1, NULL, 0, &(*p)->m_wsa_over, NULL);
	}

	// 객체 새로 추가
	std::shared_ptr<EXP_OVER> overLappedEx = make_shared<EXP_OVER>(packet);
	m_sendOverlapped_list.push_back(overLappedEx);
	return WSASend(m_fd, &overLappedEx->m_wsa_buf, 1, NULL, 0, &overLappedEx->m_wsa_over, NULL);
}

#endif

// 넌블럭 소켓으로 모드를 설정합니다.
void Socket::SetNonblocking()
{
	u_long val = 1;
#ifdef _WIN32
	int ret = ioctlsocket(m_fd, FIONBIO, &val);
#else
	int ret = ioctl(m_fd, FIONBIO, &val);
#endif
	if (ret != 0)
	{
		stringstream ss;
		ss << "bind failed:" << GetLastErrorAsString();
		throw Exception(ss.str().c_str());
	}
}

void Socket::Disconnect()
{
	try
	{
		// 먼저 shutdown을 통해서 데이터를 더 이상 보내지 않음을 알립니다.
#ifdef _WIN32
		if (shutdown(m_fd, SD_SEND) == SOCKET_ERROR)
#else
		if (shutdown(m_fd, SHUT_WR) < 0)
#endif
		{
			stringstream ss;
			ss << "shutdown failed:" << GetLastErrorAsString();
			throw Exception(ss.str().c_str());
		}

		// 서버에서 연결을 종료하는 경우 상대방이 데이터를 모두 읽고 연결을 종료할 수 있도록 recv를 통해 대기합니다.
		char buffer[1024];
		int ret;
		do
		{
			ret = recv(m_fd, buffer, sizeof(buffer), 0);
		} while (ret > 0);

		// 이제 소켓을 닫습니다.
		Close();
	}
	catch (const Exception& ex)
	{
		// 예외가 발생하더라도 소켓을 닫아야 합니다.
		Close();
		throw;
	}
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
// 출처: https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}
