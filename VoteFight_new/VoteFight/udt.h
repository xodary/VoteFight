#pragma once

// �̱��� ������ ���� �ֻ��� ��ü
template <typename T>
class CSingleton abstract
{
protected:
	// �� ��ü�� ��� ���� �ڽ� ��ü���� �θ��� �� Ŭ������ �����ڸ� ȣ���ؾ��ϹǷ�, ���� �����ڸ� protected�� �����Ͽ���.
	// �� ��ü�� ��� ���� ��ü�� �ݵ�� friend ������ ���־�� �Ѵ�.
	CSingleton()
	{
	}

	virtual ~CSingleton()
	{
	}

public:
	static T* GetInstance()
	{
		static T instance = {};

		return &instance;
	}

	virtual void Init()
	{
	}
};



// ��Ŷ ������
struct SOCKET_INFO
{
	UINT		m_ID;
	SOCKET      m_Socket;
	SOCKADDR_IN m_SocketAddress;
};

struct INIT_GAME_DATA
{
	vector<XMFLOAT4X4> m_PlayerInitTransformMatrixes;
	vector<XMFLOAT4X4> m_NPCInitTransformMatrixes;
};

struct CLIENT_TO_SERVER_DATA
{
	UINT	   m_InputMask;
	XMFLOAT4X4 m_WorldMatrix;
};

struct CAMERA_DATA
{
	XMFLOAT3 m_CameraPosition;
	XMFLOAT3 m_CameraDirection;
};

struct PLAYER_ATTACK_DATA
{
	UINT m_TargetIndices[MAX_PLAYER_CAPACITY];
};

struct GUARD_ATTACK_DATA
{
	UINT m_TargetIndices[MAX_NPC_COUNT];
};

struct TRIGGER_DATA
{
	UINT m_TargetIndices[MAX_PLAYER_CAPACITY];
};

struct SERVER_TO_CLIENT_DATA
{
	MSG_TYPE			m_MsgType;

	XMFLOAT4X4          m_PlayerWorldMatrixes[MAX_PLAYER_CAPACITY];
	ANIMATION_CLIP_TYPE m_PlayerAnimationClipTypes[MAX_PLAYER_CAPACITY];

	XMFLOAT4X4          m_NPCWorldMatrixes[MAX_NPC_COUNT];
	ANIMATION_CLIP_TYPE m_NPCAnimationClipTypes[MAX_NPC_COUNT];

	XMFLOAT3            m_TowerLightDirection;
};
