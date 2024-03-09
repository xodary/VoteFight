#pragma once

class CAsset
{
	friend class CAssetManager;

protected:
	string m_name;

protected:
	// 이 객체의 생성은 오로지 CAssetManager에 의해서만 일어난다.
	// 단, 이 객체를 상속 받은 자식 클래스의 생성자에서 이 클래스의 생성자를 호출해야하므로 접근 지정자를 protected로 설정하였다.
	CAsset();

public:
	// 소멸자의 경우에는 SafeDelete 외부 함수를 이용하기 때문에 접근 지정자를 public으로 설정하였다.
	virtual ~CAsset();

	void SetName(const string& name);
	const string& GetName();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();
};
