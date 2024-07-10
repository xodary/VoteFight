#include "ImaysNet/ImaysNet.h"
#include "CPlayer.h"

class RemoteClient {
public:
	static unordered_map < RemoteClient*, shared_ptr<RemoteClient>>	m_remoteClients;
	static recursive_mutex	m_lock;

public:
	shared_ptr<thread>		m_thread;			// Client thread
	shared_ptr<CPlayer>		m_player;
	Socket					m_tcpConnection;	// Acceept Tcp
	unsigned long long		m_id;				// Client ID
	string					m_name;				// Client Name
	atomic_bool				m_ingame;
	chrono::system_clock::time_point		m_lastTime;
	atomic_bool				m_ready;
	int						m_char = -1;

public:
	RemoteClient() : m_thread(), m_ingame(false), m_tcpConnection(SocketType::Tcp) {}
	RemoteClient(SocketType _socketType) : m_tcpConnection(_socketType) {}
};

