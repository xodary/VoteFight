#pragma once

enum class KEY
{
	UP, DOWN, LEFT, RIGHT, ALT, CTRL, SHIFT, TAB, SPACE, ENTER, ESC,
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	LBUTTON, RBUTTON,

	COUNT
};

enum class KEY_STATE
{
	NONE, // �������� ������ �ʾҰ�, ���絵 ������ ���� ����
	TAP,  // ������ ������ �ʾҰ�, ���� ���� ����
	HOLD, // ������ ���Ȱ�, ���絵 ���� ����
	AWAY, // ������ ���Ȱ�, ���� ���� ����
};

struct KeyInfo
{
	bool	  m_isPressed; // ���� �����ӿ� ���ȴ����� ���� ����
	KEY_STATE m_state;     // ���� Ű�� ����
};

// 1. ������ ����ȭ
//   - ���� ������ ������ ���� Ű�� ���ؼ�, ������ �̺�Ʈ�� �������� �Ѵ�.
// 2. Ű �Է� ���� ó��
//    - Tap, Hold, Away
class CInputManager : public CSingleton<CInputManager>
{
	friend class CSingleton;

private:
	int      m_virtualKey[(int)KEY::COUNT]; // ���� Ű ���� KEY�� ������ ��ġ�ؾ� �Ѵ�.
	KeyInfo  m_keyInfo[(int)KEY::COUNT];

	XMFLOAT2 m_cursor;
	XMFLOAT2 m_oldCursor;

private:
	CInputManager();
	~CInputManager();

public:
	KEY_STATE GetKeyState(KEY key);

	const XMFLOAT2& GetCursor();
	const XMFLOAT2& GetOldCursor();

	void Init();

	void Update();
};
