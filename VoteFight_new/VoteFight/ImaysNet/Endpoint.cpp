#include "../pch.h"
#include "Endpoint.h"

Endpoint Endpoint::Any;

Endpoint::Endpoint()
{
	memset(&m_ipv4Endpoint, 0, sizeof(m_ipv4Endpoint));
	m_ipv4Endpoint.sin_family = AF_INET;  // �ּ� ü�踦 IPv4�� ����
}

// �ּҿ� ��Ʈ�� �Է¹޾� ��������Ʈ�� �غ�
Endpoint::Endpoint(const char* _address, int _port)
{
	memset(&m_ipv4Endpoint, 0, sizeof(m_ipv4Endpoint));
	m_ipv4Endpoint.sin_family = AF_INET;
	inet_pton(AF_INET, _address, &m_ipv4Endpoint.sin_addr);
	m_ipv4Endpoint.sin_port = htons((uint16_t)_port); 
}

Endpoint::~Endpoint()
{
}

std::string Endpoint::ToString()
{
	char addrString[1000];		// �ּ� ���� ����
	addrString[0] = 0;			// ���� �ʱ�ȭ
	inet_ntop(AF_INET, &m_ipv4Endpoint.sin_addr, addrString, sizeof(addrString) - 1);	// �ּҸ� ���ڿ��� ��ȯ�Ͽ� ����
		
	char finalString[1000];		// ���� ���ڿ� ���� ����
	sprintf(finalString, "%s:%d", addrString, htons(m_ipv4Endpoint.sin_port));		// �ּҿ� ��Ʈ�� ���ļ� ���� ���ڿ��� ����

	return finalString;			// ���� ���ڿ� ��ȯ
}
