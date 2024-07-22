#include "pch.h"
#include "CPlayer.h"

CPlayer::CPlayer()
{
	m_Pos = XMFLOAT3(0.f, 0.f, 0.f);
	m_Angle = 0.0f;
	m_Rota = XMFLOAT3(0.f, 0.f, 0.f);
	m_Velocity = 0.0f;
	m_Vec = XMFLOAT3(0.f, 0.f, 0.f);
	m_Health = 100;
	m_dead = false;
	m_Weapon = 0;
}

CPlayer::CPlayer(XMFLOAT3 _pos)
{
	m_Pos = _pos;
	m_Angle = 0.0f;
	m_Rota = XMFLOAT3(0.f, 0.f, 0.f);
	m_Velocity = 0.0f;
	m_Vec = XMFLOAT3(0.f, 0.f, 0.f);
	m_Health = 100;
	m_dead = false;
	m_Weapon = 0;
}

CPlayer::~CPlayer()
{

}