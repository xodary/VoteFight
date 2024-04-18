#pragma once

#ifdef _WIN32
#include <Ws2tcpip.h>
#else 
#include <netinet/in.h>
#include <netinet/ip.h>
#endif

// ��Ʈ��ũ ��ſ��� ���Ǵ� ��������Ʈ
class Endpoint {
public:
	sockaddr_in			m_ipv4Endpoint;		// IPv4 �ּҿ� ��Ʈ ��ȣ�� �����ϱ� ���� ����ü
	static Endpoint		Any;				// ��� �ּҸ� ��Ÿ���� Ư���� ��������Ʈ

public:
	Endpoint();
	Endpoint(const char* _address, int _port);
	~Endpoint();

	std::string			ToString();			// ���� ��������Ʈ�� �ּҿ� ��Ʈ ��ȣ�� ���ڿ��� ��ȯ
};

