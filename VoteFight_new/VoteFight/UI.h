#pragma once
#include "Object.h"

class CUI : public CObject
{
private:
	bool m_isFixed;      // 카메라의 영향을 받지 않아 위치가 고정적인지에 대한 여부
	bool m_isCursorOver; // 현재 커서의 위치가 이 객체 위에 있는지에 대한 여부
	
	int						 m_vertexCount;

	ComPtr<ID3D12Resource>	 m_d3d12VertexBuffer;
	ComPtr<ID3D12Resource>	 m_d3d12VertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_d3d12VertexBufferView;

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

class CBilboardUI : public CUI
{
public:
	CObject* m_owner;

	CBilboardUI();
	~CBilboardUI();

	virtual CComponent* CreateComponent(COMPONENT_TYPE componentType);
};

class CHPbarUI : public CBilboardUI
{
public:
	CHPbarUI(CObject* owner);
	~CHPbarUI() { 
		cout << "deleted" << endl; }
	virtual void Update();
};

class CSpeechBubbleUI : public CBilboardUI
{
public:
	CSpeechBubbleUI(CObject* owner);
	~CSpeechBubbleUI() {
		cout << "deleted" << endl;
	}
	virtual void Update();
};

class CTextUI : public CBilboardUI
{
public:

	CTextUI(CObject* owner);
	~CTextUI() {
		cout << "deleted" << endl; }

	virtual void Render(CCamera* camera);
	virtual void Update();
};