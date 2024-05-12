#include "CPlayer.h"

CPlayer::CPlayer() : m_Xpos{ 0 }, m_Ypos{ 0 }, m_Zpos{ 0 }
{
}

CPlayer::CPlayer(float _xPos, float _yPos, float _zPos)
{
	m_Xpos = _xPos;
	m_Ypos = _yPos;
	m_Zpos = _zPos;
}

CPlayer::~CPlayer()
{

}