#include "../pch.h"
#include "SocketInit.h"

SocketInit g_socketInit;

SocketInit::SocketInit()
{
#ifdef _WIN32
	// Windows������ WSAStartup, WSACleanup�� ���ʿ� ���Ŀ� �� �ѹ� �־�� �մϴ�.
	WSADATA w;
	WSAStartup(MAKEWORD(2, 2), &w);
#endif

}

void SocketInit::Touch()
{
}

