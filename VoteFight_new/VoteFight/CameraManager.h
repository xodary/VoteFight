#pragma once

class CCamera;

class CCameraManager : public CSingleton<CCameraManager>
{
	friend class CSingleton;

private:
	vector<CCamera*> m_cameras;

private:
	CCameraManager();
	virtual ~CCameraManager();

public:
	CCamera* GetMainCamera();
	CCamera* GetUICamera();

	const vector<CCamera*>& GetCameras();

	virtual void Init();

	void Update();
};
