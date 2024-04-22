#pragma once

// 예외 처리
class Exception : public std::exception {
public:
	std::string		m_text;								// 예외에 관한 메시지를 저장하기 위한 변수

public:
	Exception(const std::string& text);
	~Exception();

	const char*		what() { return m_text.c_str(); }	// 예외가 발생했을 때 예외 메시지를 반환
};

