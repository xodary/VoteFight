#pragma once 

class SocketInit {
public:
	SocketInit();

	// NOTE: WSACleanup�� ȣ���ϴ� ������ �־�� �ϴ� ���� �Ϲ����Դϴ�.
	// �׷���, C++���� ���� ��ü�� �ı��� ȣ���� ������ 
	// �� �� �����Ƿ�, ������ ���μ����� �����ϴ� �����̹Ƿ� ���� WSACleanup�� ȣ�������� �ʽ��ϴ�.
	void	Touch();
};

extern SocketInit g_socketInit;