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
	P_CS_WALK_ENTER_PACKET,
	P_CS_MOVE_PACKET,

	P_CS_MOVE_V_PACKET,

	// Server -> Client packet
	P_SC_LOGIN_OK_PACKET,
	P_SC_ADD_PACKET,
	P_SC_WALK_ENTER_INFO_PACKET,

	P_SC_MOVE_V_PACKET,
};

#pragma pack (push, 1)

// Packet(Server->Client)
struct SC_LOGIN_OK_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	float				m_xPos;
	float				m_yPos;
	float				m_zPos;
};

struct SC_ADD_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	float				m_xPos;
	float				m_yPos;
	float				m_zPos;
};

struct SC_WALK_ENTER_INFO_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	std::string			m_key;
	float				m_maxSpeed;
	float				m_vel;
};

struct SC_MOVE_V_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_vec;
	XMFLOAT3			m_rota;
	//int				m_state;
};


// Packet(Clinet->Server)
struct CS_LOGIN_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct CS_WALK_ENTER_PACEKET {
	unsigned char		m_size;
	unsigned char		m_type;	
};

struct CS_MOVE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct CS_MOVE_V_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_vec;
	XMFLOAT3			m_rota;
	//int					m_state;
};

#pragma pack (pop)