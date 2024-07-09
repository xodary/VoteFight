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
	P_CS_VELOCITY_CHANGE_PACKET,
	P_CS_MOVE_PACKET,
	P_CS_ANIMATION,
	P_CS_MOVE_V_PACKET,
	P_CS_KEYDOWN_PACKET,
	P_CS_WALK_ENTER_PACKET,
	P_CS_STOP_PACKET,

	// Server -> Client packet
	P_SC_LOGIN_OK_PACKET,
	P_SC_ADD_PACKET,
	P_SC_VELOCITY_CHANGE_PACKET,
	P_SC_ANIMATION_PACKET,
	P_SC_MOVE_V_PACKET,
	P_SC_POS_PACKET
};

#pragma pack (push, 1)

// Packet(Server->Client)
struct SC_LOGIN_OK_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
};

struct SC_ANIMATION_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	std::string			m_key;
};

struct SC_ADD_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	char				m_name[NAME_SIZE];
	XMFLOAT3			m_pos;
};

struct SC_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
	float				m_vel;
	float				m_rota;
};

struct SC_MOVE_V_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_vec;
	XMFLOAT3			m_rota;
	STATE_ENUM			m_state;
};

struct SC_POS_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
	float				m_rota;
};

// Packet(Clinet->Server)
struct CS_LOGIN_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	char				m_name[NAME_SIZE];
};

struct CS_ANIMATION_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	std::string			m_key;
};

struct CS_WALK_ENTER_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	WEAPON_TYPE			m_weapon;
};

struct CS_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	float				m_rota;
	bool				m_shift;
	XMFLOAT3			m_pos;
};


struct CS_MOVE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	XMFLOAT3			m_vel;
	float				m_rota;
};

struct CS_MOVE_V_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_vec;
	float				m_rota;
	STATE_ENUM			m_state;
};

struct CS_KEYDOWN_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct CS_STOP_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

#pragma pack (pop)