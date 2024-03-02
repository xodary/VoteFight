#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define IDLE					0
#define TURNRIGHT				1
#define TURNLEFT				2
#define WALK					3
#define RUN						4

#define LOWER_ANIMATION			5

#define TURNSPEED				4.0f

#include "Object.h"
#include "Camera.h"

namespace playerState {
	// state
	enum class State { Idle = 0, TurnRight, TurnLeft, Walk, Run };

	/* 
		Player의 Animation State를 나타내는 클래스 (애니메이션 트리)
		2024-02-21 13:43 황유림 수정
	*/


	class PlayerState abstract {
	public:
		CPlayer* m_pPlayer = nullptr;
		XMFLOAT3 m_xmf3Direction = XMFLOAT3(0, 0, 1);
		XMFLOAT3 m_xmf3Origin = XMFLOAT3(0, 0, 1);
		int m_nTurn = 0;

		PlayerState(CPlayer* player) { m_pPlayer = player; }
		virtual void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0,0,0)) abstract;
		virtual void Exit(const State& playerState) abstract;
		virtual void Update(float fTimeElapsed) abstract;
		virtual void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) abstract;
	};

	class Idle : public PlayerState {
	public:
		Idle(CPlayer* player) : PlayerState(player) {};
		void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0, 0, 0)) override;
		void Exit(const State& playerState) override;
		void Update(float fTimeElapsed) override;
		void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) override;
	};

	class Walk : public PlayerState {
	public:
		Walk(CPlayer* player) : PlayerState(player) {};
		void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0, 0, 0)) override;
		void Exit(const State& playerState) override;
		void Update(float fTimeElapsed) override;
		void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) override;
	};

	class TurnRight : public PlayerState {
	public:
		TurnRight(CPlayer* player) : PlayerState(player) {};
		void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0, 0, 0)) override;
		void Exit(const State& playerState) override;
		void Update(float fTimeElapsed) override;
		void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) override;
	};

	class TurnLeft : public PlayerState {
	public:
		TurnLeft(CPlayer* player) : PlayerState(player) {};
		void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0, 0, 0)) override;
		void Exit(const State& playerState) override;
		void Update(float fTimeElapsed) override;
		void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) override;
	};

	class Run : public PlayerState {
	public:
		Run(CPlayer* player) : PlayerState(player) {};
		void Enter(const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0, 0, 0)) override;
		void Exit(const State& playerState) override;
		void Update(float fTimeElapsed) override;
		void HandleEvent(const Event& e, const XMFLOAT3& xmf3Direction, const WPARAM& wParam) override;
	};

}


class CPlayer : public CGameObject
{
public:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3					m_xmf3LookEnd = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fFullAngle = 0.0f;
	float						m_fAngle = 0.0f;

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;

	float						walk = 0.0f;
	float						idle = 1.0f;

	bool						m_bMoving = false;
	bool						m_bBlending = false;
	
	// state
	playerState::PlayerState*	m_pCrntState = nullptr;

	int							m_nUpState = -1;
	float						m_fStates[LOWER_ANIMATION] = { 1.0f };

public:
	CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext = NULL);
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }

	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	void SetLookEnd(const XMFLOAT3& xmf3Look) { m_xmf3LookEnd = xmf3Look; }

	void Move(DWORD xmf3Direction, float fDistance, bool bUpdateVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);

	void Rotate(float x, float y, float z);

	virtual void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed);
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual CCamera* CreateCamera(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	void ChangeState(const playerState::State& playerState, const Event& e = Event::None, const XMFLOAT3& xmf3Direction = XMFLOAT3(0,0,0));

	void BlendAnimation(float fTimeElapsed);
};

class CSoundCallbackHandler : public CAnimationCallbackHandler
{
public:
	CSoundCallbackHandler() { }
	~CSoundCallbackHandler() { }

public:
	virtual void HandleCallback(void *pCallbackData, float fTrackPosition); 
};