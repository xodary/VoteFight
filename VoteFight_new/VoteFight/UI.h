#pragma once
#include "Object.h"

class CUI : public CObject
{
private:
	bool m_isFixed;      // 카메라의 영향을 받지 않아 위치가 고정적인지에 대한 여부
	bool m_isCursorOver; // 현재 커서의 위치가 이 객체 위에 있는지에 대한 여부

public:
	CUI();
	virtual ~CUI();
	
	static CUI* Load(ifstream& in);

	void SetFixed(bool isFixed);
	bool IsFixed();

	bool IsCursorOver();

	virtual CComponent* CreateComponent(COMPONENT_TYPE componentType);

	virtual void OnCursorOver();		    // 현재 커서의 위치가 이 UI 위에 있을 때 호출
	virtual void OnCursorLeftButtonDown();  // 현재 커서의 위치가 이 UI 위에 있고, 왼쪽 마우스 버튼이 눌렸을 때 호출
	virtual void OnCursorLeftButtonUp();    // 현재 커서의 위치에 상관 없이, 이 UI가 이전에 눌렸고 현재 프레임에 때졌을 때 호출
	virtual void OnCursorLeftButtonClick(); // 현재 커서의 위치가 이 UI 위에 있고, 이전에 눌렸으며 현재 프레임에 때졌을 때 호출

	virtual void Update();

	virtual void Render(CCamera* camera);

private:
	void CheckCursorOver();
};

//=========================================================================================================================

class CMissionUI : public CUI
{
public:
	CMissionUI();
	virtual ~CMissionUI();

	virtual void Init();
};

//=========================================================================================================================

class CKeyUI : public CUI
{
public:
	CKeyUI();
	virtual ~CKeyUI();
};

//=========================================================================================================================

class CHitUI : public CUI
{
public:
	CHitUI();
	virtual ~CHitUI();

	virtual void Init();
};
