#include "pch.h"
#include "CPlayer.h"

CPlayer::CPlayer()
{
	m_Pos = XMFLOAT3(0.f, 0.f, 0.f);
	m_Rota = 0.0f;
	m_Velocity = 0.0f;
	m_Vec = XMFLOAT3(0.f, 0.f, 0.f);
}

CPlayer::CPlayer(XMFLOAT3 _pos)
{
	m_Pos = _pos;
	m_Rota = 0.0f;
	m_Velocity = 0.0f;
	m_Vec = XMFLOAT3(0.f, 0.f, 0.f);
}

CPlayer::~CPlayer()
{

}