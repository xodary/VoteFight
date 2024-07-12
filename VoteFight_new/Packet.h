#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Protocol.h"

constexpr int NAME_SIZE = 30;
constexpr int CHAT_SIZE = 30;

constexpr int MAX_BUFSIZE_CLIENT = 1024;

enum PACKET_TYPE {
	P_NONE = 0,

	// Client -> Server packet
	P_CS_LOGIN_PACKET,
	P_CS_VELOCITY_CHANGE_PACKET,
	P_CS_MOVE_PACKET,
	P_CS_ANIMATION,
	P_CS_WALK_ENTER_PACKET,
	P_CS_STOP_PACKET,
	P_CS_SELECT_PACKET,
	P_CS_ATTACK_PACKET,

	// Server -> Client packet
	P_SC_LOGIN_OK_PACKET,
	P_SC_SPAWN_PACKET,
	P_SC_ADD_PACKET,
	P_SC_VELOCITY_CHANGE_PACKET,
	P_SC_ANIMATION_PACKET,
	P_SC_POS_PACKET,
	P_SC_SELECT_PACKET,
	P_SC_GAMESTART_PACKET,
	P_SC_CHANGE_STATE,
	P_SC_NPC_EXCHANGE_PACKET,
	P_SC_UPDATE_PHASE_PACKET,
};

#pragma pack (push, 1)

// Packet(Server->Client)
struct SC_LOGIN_OK_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
};

struct SC_SPAWN_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
};

struct SC_GAMESTART_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct SC_SELECT_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_char;
	char				m_name[NAME_SIZE];
};

struct SC_ANIMATION_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_grouptype;
	unsigned int		m_speed;
	char				m_key[NAME_SIZE];
};

struct SC_ADD_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_grouptype;
	char				m_modelName[NAME_SIZE];
	XMFLOAT3			m_pos;
	XMFLOAT3			m_rota;
	XMFLOAT3			m_sca;
};

struct SC_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
	unsigned int		m_grouptype;
	float				m_vel;
	float				m_angle;
};

struct SC_POS_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_grouptype;
	XMFLOAT3			m_pos;
	float				m_angle;
};

struct SC_CHANGE_STATE {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	int					m_health;
};

struct SC_NPC_EXCHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	int					m_id;
	int					m_itemType;	// 0�̸� need, 1�̸� output
	char				m_itemName[NAME_SIZE];
};

struct SC_UPDATE_PHASE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	int					m_phase;
	chrono::seconds		m_time;
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
	char				m_key[NAME_SIZE];
};

struct CS_WALK_ENTER_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_weapon;
};

struct CS_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	float				m_angle;
	bool				m_shift;
	XMFLOAT3			m_pos;
};

struct CS_MOVE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	XMFLOAT3			m_vel;
	float				m_angle;
};

struct CS_STOP_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	XMFLOAT3			m_pos;
};

struct CS_SELECT_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_char;
};

struct CS_ATTACK_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_weapon;
	XMFLOAT3			m_vec;
};


#pragma pack (pop)