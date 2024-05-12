#include "pch.h"
#include "OtherPlayerStates.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Player.h"
#include "StateMachine.h"
#include "RigidBody.h"
#include "Animator.h"
#include "Transform.h"
#include "GameFramework.h"
#include <bitset>

#include "./ImaysNet/ImaysNet.h"
#include "./ImaysNet/PacketQueue.h"

COtherPlayerIdleState::COtherPlayerIdleState()
{
}

COtherPlayerIdleState::~COtherPlayerIdleState()
{
}

void COtherPlayerIdleState::Enter(CObject* object)
{
	stateNum = STATE_ENUM::CPlayerIdleState;
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("idle", true);
}

void COtherPlayerIdleState::Exit(CObject* object)
{
}

void COtherPlayerIdleState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));
	// cout << transform->GetRotation().y << endl;
}

COtherPlayerLeftTurn::COtherPlayerLeftTurn()
	: restAngle(90.f),
	lookAngle()
{
}

COtherPlayerLeftTurn::~COtherPlayerLeftTurn()
{
}

void COtherPlayerLeftTurn::Enter(CObject* object)
{
	stateNum = STATE_ENUM::CPlayerLeftTurn;

	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);

	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = transform->GetRotation().y - player->GetTurnAngle();
	lookAngle = player->GetClickAngle();
	if (restAngle > 180) restAngle -= 180;
	else if (restAngle <= -270) restAngle += 360;
}

void COtherPlayerLeftTurn::Exit(CObject* object)
{
}

void COtherPlayerLeftTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 180.f * DT;
	if (restAngle < 0)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(COtherPlayerIdleState::GetInstance());
		transform->SetRotation(XMFLOAT3(0, player->GetTurnAngle(), 0));
	}
	else {
		float angle = transform->GetRotation().y - 180.f * DT;
		// cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}

}

COtherPlayerRightTurn::COtherPlayerRightTurn()
	: restAngle(90.f),
	lookAngle()
{
}

COtherPlayerRightTurn::~COtherPlayerRightTurn()
{
}

void COtherPlayerRightTurn::Enter(CObject* object)
{
	stateNum = STATE_ENUM::CPlayerRightTurn;

	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("turnLeft", false);
	
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle = player->GetTurnAngle() - transform->GetRotation().y;
	lookAngle = player->GetClickAngle();
	if (restAngle > 180) restAngle -= 180;
	else if (restAngle <= -270) restAngle += 360;
}

void COtherPlayerRightTurn::Exit(CObject* object)
{
}

void COtherPlayerRightTurn::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));
	restAngle -= 180.f * DT;
	if (restAngle < 0)
	{
		CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
		stateMachine->ChangeState(COtherPlayerIdleState::GetInstance());
		transform->SetRotation(XMFLOAT3(0, player->GetTurnAngle(), 0));
	}
	else {
		float angle = transform->GetRotation().y + 180.f * DT;
		// cout << angle << endl;
		transform->SetRotation(XMFLOAT3(0, angle, 0));
	}

}

COtherPlayerWalkState::COtherPlayerWalkState()
{
}

COtherPlayerWalkState::~COtherPlayerWalkState()
{
}

void COtherPlayerWalkState::Enter(CObject* object)
{
	//CS_WALK_ENTER_PACEKET send_packet;
	//send_packet.m_size = sizeof(CS_WALK_ENTER_PACEKET);
	//send_packet.m_type = PACKET_TYPE::P_CS_WALK_ENTER_PACKET;
	//PacketQueue::AddSendPacket(&send_packet);
	// cout << " >> send ) CS_WALK_ENTER_PACEKET" << endl;

	stateNum = STATE_ENUM::CPlayerWalkState;
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("lisaWalk", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(400.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 400.0f * DT));
}

void COtherPlayerWalkState::Exit(CObject* object)
{
}

void COtherPlayerWalkState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));

	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	float angle = -45;


	if (Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(COtherPlayerIdleState::GetInstance());
		return;
	}

	CAnimator* animator = static_cast<CAnimator*>(player->GetComponent(COMPONENT_TYPE::ANIMATOR));
	XMFLOAT3 direction = Vector3::Normalize(rigidBody->GetVelocity());

}

COhterPlayerRunState::COhterPlayerRunState()
{
}

COhterPlayerRunState::~COhterPlayerRunState()
{
}

void COhterPlayerRunState::Enter(CObject* object)
{
	stateNum = STATE_ENUM::CPlayerRunState;
	CAnimator* animator = static_cast<CAnimator*>(object->GetComponent(COMPONENT_TYPE::ANIMATOR));
	animator->Play("Run", true);

	CRigidBody* rigidBody = static_cast<CRigidBody*>(object->GetComponent(COMPONENT_TYPE::RIGIDBODY));
	CTransform* transform = static_cast<CTransform*>(object->GetComponent(COMPONENT_TYPE::TRANSFORM));

	rigidBody->SetMaxSpeedXZ(700.0f);
	rigidBody->AddVelocity(Vector3::ScalarProduct(transform->GetForward(), 700.0f * DT));

}

void COhterPlayerRunState::Exit(CObject* object)
{
}

void COhterPlayerRunState::Update(CObject* object)
{
	CPlayer* player = static_cast<CPlayer*>(object);
	CStateMachine* stateMachine = static_cast<CStateMachine*>(player->GetComponent(COMPONENT_TYPE::STATE_MACHINE));
	CRigidBody* rigidBody = static_cast<CRigidBody*>(player->GetComponent(COMPONENT_TYPE::RIGIDBODY));

	CTransform* transform = static_cast<CTransform*>(player->GetComponent(COMPONENT_TYPE::TRANSFORM));

	if (Math::IsZero(rigidBody->GetSpeedXZ()))
	{
		stateMachine->ChangeState(COtherPlayerIdleState::GetInstance());
		return;
	}

}
