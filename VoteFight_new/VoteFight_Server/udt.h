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
