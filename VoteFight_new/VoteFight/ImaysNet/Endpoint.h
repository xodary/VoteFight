#pragma once

#ifdef _WIN32
#include <Ws2tcpip.h>
#else 
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

// 네트워크 통신에서 사용되는 엔드포인트
class Endpoint {
public:
	sockaddr_in			m_ipv4Endpoint;		// IPv4 주소와 포트 번호를 저장하기 위한 구조체
	static Endpoint		Any;				// 모든 주소를 나타내는 특수한 엔드포인트

public:
	Endpoint();
	Endpoint(const char* _address, int _port);
	~Endpoint();

	std::string			ToString();			// 현재 엔드포인트의 주소와 포트 번호를 문자열로 반환
};

