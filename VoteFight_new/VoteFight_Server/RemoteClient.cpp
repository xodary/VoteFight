#include "pch.h"
#include "../VoteFight_IOCP/ImaysNet/ImaysNet.h"
#include "RemoteClient.h"

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> RemoteClient::m_remoteClients;