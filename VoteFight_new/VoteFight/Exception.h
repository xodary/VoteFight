#pragma once

// ���� ó��
class Exception : public std::exception {
public:
	std::string		m_text;								// ���ܿ� ���� �޽����� �����ϱ� ���� ����

public:
	Exception(const std::string& text);
	~Exception();

	const char*		what() { return m_text.c_str(); }	// ���ܰ� �߻����� �� ���� �޽����� ��ȯ
};

