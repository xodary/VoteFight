#pragma once

// 싱글톤 패턴을 위한 최상위 객체
template <typename T>
class CSingleton abstract
{
protected:
	// 이 객체를 상속 받은 자식 객체에서 부모인 이 클래스의 생성자를 호출해야하므로, 접근 지정자를 protected로 설정하였다.
	// 이 객체를 상속 받은 객체는 반드시 friend 선언을 해주어야 한다.
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
