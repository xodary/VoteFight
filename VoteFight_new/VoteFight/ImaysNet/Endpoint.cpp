#include "../pch.h"
#include "Endpoint.h"

Endpoint Endpoint::Any;

Endpoint::Endpoint()
{
	memset(&m_ipv4Endpoint, 0, sizeof(m_ipv4Endpoint));
	m_ipv4Endpoint.sin_family = AF_INET;  // 주소 체계를 IPv4로 설정
}

// 주소와 포트를 입력받아 엔드포인트를 준비
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
	char addrString[1000];		// 주소 저장 버퍼
	addrString[0] = 0;			// 버퍼 초기화
	inet_ntop(AF_INET, &m_ipv4Endpoint.sin_addr, addrString, sizeof(addrString) - 1);	// 주소를 문자열로 변환하여 저장
		
	char finalString[1000];		// 최종 문자열 저장 버퍼
	sprintf(finalString, "%s:%d", addrString, htons(m_ipv4Endpoint.sin_port));		// 주소와 포트를 합쳐서 최종 문자열을 생성

	return finalString;			// 최종 문자열 반환
}
