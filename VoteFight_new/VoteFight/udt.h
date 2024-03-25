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
