#include "pch.h"
#include "Exception.h"

Exception::Exception(const std::string& _text)
{
	m_text = _text;
}

Exception::~Exception()
{
}
