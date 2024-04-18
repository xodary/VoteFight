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
	NONE, // 이전에도 눌리지 않았고, 현재도 눌리지 않은 상태
	TAP,  // 이전에 눌리지 않았고, 현재 눌린 상태
	HOLD, // 이전에 눌렸고, 현재도 눌린 상태
	AWAY, // 이전에 눌렸고, 현재 떼진 상태
};

struct KeyInfo
{
	bool	  m_isPressed; // 이전 프레임에 눌렸는지에 대한 여부
	KEY_STATE m_state;     // 현재 키의 상태
};

// 1. 프레임 동기화
//   - 동일 프레임 내에서 같은 키에 대해서, 동일한 이벤트를 가져가야 한다.
// 2. 키 입력 상태 처리
//    - Tap, Hold, Away
class CInputManager : public CSingleton<CInputManager>
{
	friend class CSingleton<CInputManager>;

private:
	int      m_virtualKey[(int)KEY::COUNT]; // 가상 키 값은 KEY의 순서와 일치해야 한다.
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
