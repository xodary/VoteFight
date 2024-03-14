#include "pch.h"
#include "InputManager.h"
#include "GameFramework.h"

CInputManager::CInputManager() :
	m_virtualKey{ VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_MENU, VK_CONTROL, VK_LSHIFT, VK_TAB, VK_SPACE, VK_RETURN, VK_ESCAPE, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', VK_LBUTTON, VK_RBUTTON },
	m_keyInfo{},
	m_cursor(),
	m_oldCursor()
{
}

CInputManager::~CInputManager()
{
}

KEY_STATE CInputManager::GetKeyState(KEY key)
{
	return m_keyInfo[(int)key].m_state;
}

const XMFLOAT2& CInputManager::GetCursor()
{
	return m_cursor;
}

const XMFLOAT2& CInputManager::GetOldCursor()
{
	return m_oldCursor;
}

void CInputManager::Init()
{
	for (int i = 0; i < (int)KEY::COUNT; ++i)
	{
		m_keyInfo[i].m_isPressed = false;
		m_keyInfo[i].m_state = KEY_STATE::NONE;
	}
}

void CInputManager::Update()
{
}
