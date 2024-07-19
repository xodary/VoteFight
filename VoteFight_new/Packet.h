#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Protocol.h"

constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 30;

constexpr int MAX_BUFSIZE_CLIENT = 1024;

enum PACKET_TYPE {
	P_NONE = 0,

	// Client -> Server packet
	P_CS_LOGIN_PACKET,
	P_CS_VELOCITY_CHANGE_PACKET,
	P_CS_MOVE_PACKET,
	P_CS_ANIMATION,
	P_CS_STATE_ENTER_PACKET,
	P_CS_SELECT_PACKET,
	P_CS_ATTACK_PACKET,
	P_CS_EXCHANGE_DONE_PACKET,
	P_CS_PLAYER_RBUTTON_PACKET,
	P_CS_TAKEOUT_PACKET,
	P_CS_WEAPON_CHANGE_PACKET,

	// Server -> Client packet
	P_SC_LOGIN_OK_PACKET,
	P_SC_LOGIN_FAIL_PACKET,
	P_SC_SPAWN_PACKET,
	P_SC_ADD_PACKET,
	P_SC_DELETE_PACKET,
	P_SC_VELOCITY_CHANGE_PACKET,
	P_SC_ANIMATION_PACKET,
	P_SC_POS_PACKET,
	P_SC_SELECT_PACKET,
	P_SC_GAMESTART_PACKET,
	P_SC_CHANGE_STATE,
	P_SC_NPC_EXCHANGE_PACKET,
	P_SC_UPDATE_PHASE_PACKET,
	P_SC_EXCHANGE_DONE_PACKET,
	P_SC_PLAYER_RBUTTON_PACKET,
	P_SC_HEALTH_CHANGE_PACKET,
	P_SC_TAKEOUT_PACKET,
	P_SC_MONSTER_DEAD_PACKET,
	P_SC_TREE_PACKET,
	P_SC_WEAPON_CHANGE_PACKET,
};

#pragma pack (push, 1)

// Packet(Server->Client)
struct SC_LOGIN_OK_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
};

struct SC_LOGIN_FAIL_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_fail_type;
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
	char				m_key[NAME_SIZE];
	bool				m_loop;
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

struct SC_DELETE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_groupType;
	unsigned int		m_itemID;
};

struct SC_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	XMFLOAT3			m_pos;
	unsigned int		m_grouptype;
	unsigned int		m_keytype;
	float				m_vel;
	float				m_angle;
	float				m_look;
};

struct SC_PLAYER_RBUTTON_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
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
	int					m_itemType;	// 0이면 need, 1이면 output
	char				m_itemName[NAME_SIZE];
};

struct SC_UPDATE_PHASE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	int					m_phase;
	chrono::seconds		m_time;
};

struct SC_EXCHANGE_DONE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_npc_id;
};

struct SC_HEALTH_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_groupType;
	int					m_health;
};

struct SC_TAKEOUT_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_itemID;
};

struct SC_TREE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
};

struct SC_MONSTER_DEAD_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
};

struct SC_WEAPON_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_id;
	unsigned int		m_weapon;
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

struct CS_VELOCITY_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	float				m_angle;
	float				m_look;
	XMFLOAT3			m_pos;
	bool				m_shift; 
	bool				m_Rbutton;
};

struct CS_MOVE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	XMFLOAT3			m_vel;
	float				m_angle;
};

struct CS_STATE_ENTER_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_state;
	unsigned int		m_weapon;
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
	float				m_angle;
	XMFLOAT3			m_pos;
	bool				m_Rbutton;
};

struct CS_EXCHANGE_DONE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_npc_id;
};

struct CS_TAKEOUT_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_groupType;
	unsigned int		m_itemID;
};

struct CS_PLAYER_RBUTTON_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	float				m_angle;
};

struct CS_WEAPON_CHANGE_PACKET {
	unsigned char		m_size;
	unsigned char		m_type;
	unsigned int		m_weapon;
};
#pragma pack (pop)