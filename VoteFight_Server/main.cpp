#include "stdafx.h"
#include "Server.h"

int main()
{
	CServer Server{};

	while (true)
	{
		int userInput{};

		cout << "1�� �Է��ϸ� ���ν����尡 ����˴ϴ�." << endl;
		cin >> userInput;

		if (userInput == 1)
		{
			break;
		}
	}

	return 0;
}