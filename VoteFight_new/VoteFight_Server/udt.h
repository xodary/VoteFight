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

// Packet
struct SOCKET_INFO
{
	UINT		m_ID{};
	SOCKET      m_Socket{};
	SOCKADDR_IN m_SocketAddress{};
};

struct CLIENT_TO_SERVER_DATA
{
	UINT	   m_SceneState{};

	UINT	   m_InputMask{};
	XMFLOAT4X4 m_WorldMatrix{};
};

struct SERVER_TO_CLIENT_DATA
{
	MSG_TYPE			m_MsgType;

	XMFLOAT4X4          m_PlayerWorldMatrixes[MAX_CLIENT_CAPACITY]{};
	//ANIMATION_CLIP_TYPE m_PlayerAnimationClipTypes[MAX_CLIENT_CAPACITY]{};
};
