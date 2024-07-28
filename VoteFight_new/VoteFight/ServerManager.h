#pragma once
class Socket;

class CServerManager : public CSingleton<CServerManager>
{
	friend class CSingleton<CServerManager>;

private:
	static recursive_mutex		m_mutex;

public:
	static shared_ptr<Socket>	m_tcpSocket;

	static char*				m_SERVERIP;
	static int					m_id;


	static bool					m_isLogin;

public:
	static bool					ConnectServer(string server_s);
	static void					Tick();
	static bool					Connetion();
	static void					Do_Recv();
	static void					Do_Send(const char* _buf, short _buf_size);
	static void					PacketProcess(char* _Packet);
};

