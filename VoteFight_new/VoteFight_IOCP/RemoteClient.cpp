#include "pch.h"
#include "RemoteClient.h"

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> RemoteClient::m_remoteClients;
recursive_mutex RemoteClient::m_lock;