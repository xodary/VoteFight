#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Protocol.h"

constexpr int NAME_SIZE = 15;
constexpr int CHAT_SIZE = 30;

constexpr int MAX_BUFSIZE_CLIENT = 1024;

enum PACKET_TYPE {
	P_NONE = 0,

	// Client -> Server packet
	P_CS_LOGIN_PACKET,
	P_CS_MOVE_PACKET,

	// Server -> Client packet
	P_SC_INIT_PACKET,
	P_SC_ADD_PACKET
};

#pragma pack (push, 1)

// Packet(Server->Client)

struct SC_INIT_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
};

struct SC_ADD_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
};

// Packet(Clinet->Server)

struct CS_LOGIN_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct CS_MOVE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

#pragma pack (pop)