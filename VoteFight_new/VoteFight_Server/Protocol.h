#define SERVER_IP "127.0.0.1"

const int SERVER_PORT = 9000;
const int BUFSIZE = 24;

constexpr int MAX_BUFSIZE_CLIENT = 1024;

// ±âÁ¸ MSG_TYPE -> client?
enum MSG_TYPE {
	P_NONE = 0,

	// Client -> Server packet
	P_CS_MOVE_PACKET,

	// Server -> Client packet
	P_SC_INIT_PACKET
};

// Packet
class HEAD{
public:
	unsigned char	m_size;
	unsigned char	m_type;
};

class SC_INIT_PACKET : public HEAD {
public:
	unsigned int	m_ID;
};
#pragma pack (push, 1)
