#pragma once
#include "Object.h"

class CUI : public CObject
{
private:
	bool m_isFixed;      // ī�޶��� ������ ���� �ʾ� ��ġ�� ������������ ���� ����
	bool m_isCursorOver; // ���� Ŀ���� ��ġ�� �� ��ü ���� �ִ����� ���� ����
	
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

	virtual void OnCursorOver();		    // ���� Ŀ���� ��ġ�� �� UI ���� ���� �� ȣ��
	virtual void OnCursorLeftButtonDown();  // ���� Ŀ���� ��ġ�� �� UI ���� �ְ�, ���� ���콺 ��ư�� ������ �� ȣ��
	virtual void OnCursorLeftButtonUp();    // ���� Ŀ���� ��ġ�� ��� ����, �� UI�� ������ ���Ȱ� ���� �����ӿ� ������ �� ȣ��
	virtual void OnCursorLeftButtonClick(); // ���� Ŀ���� ��ġ�� �� UI ���� �ְ�, ������ �������� ���� �����ӿ� ������ �� ȣ��

	virtual void Update();

	virtual void Render(CCamera* camera);

private:
	void CheckCursorOver();
};