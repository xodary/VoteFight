#include "pch.h"
#include "RemoteClient.h"
#include "Timer.h"
#include "GameScene.h"

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> RemoteClient::m_remoteClients;
recursive_mutex RemoteClient::m_lock;