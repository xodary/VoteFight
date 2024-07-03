#include "pch.h"
#include "Transform.h"
#include "TimeManager.h"
#include "DropItem.h"

void CDropItem::Update()
{
	CTransform* transform = reinterpret_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	transform->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	transform->Rotate(XMFLOAT3(0, DT * 1.0f, 0));

	CObject::Update();
}
