#include "ImaysNet/ImaysNet.h"

class RemoteClient{
public:
	static unordered_map < RemoteClient*, shared_ptr<RemoteClient>>	m_remoteClients;
	static recursive_mutex	m_lock;

public:
	shared_ptr<thread>		m_thread;			// Client thread
	Socket					m_tcpConnection;	// Acceept Tcp
	unsigned long long		m_id;				// Client ID

public:
	RemoteClient() : m_thread(), m_tcpConnection(SocketType::Tcp) {}
	RemoteClient(SocketType _socketType) : m_tcpConnection(_socketType) {}
};

