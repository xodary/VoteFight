#include "stdafx.h"

namespace Server
{
	void ErrorQuit(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		MessageBox(NULL, (LPCTSTR)MsgBuffer, (LPCTSTR)Msg, MB_ICONERROR);

		LocalFree(MsgBuffer);
		exit(1);
	}

	void ErrorDisplay(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		cout << "[" << Msg << "] " << (char*)MsgBuffer;

		LocalFree(MsgBuffer);
	}
}

