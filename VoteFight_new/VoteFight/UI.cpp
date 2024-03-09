#include "pch.h"
#include "UI.h"

#include "Core.h"

#include "AssetManager.h"
#include "InputManager.h"

#include "Mesh.h"
#include "Material.h"

#include "StateMachine.h"
#include "Animator.h"
#include "Transform.h"
#include "SpriteRenderer.h"

#include "UIStates.h"

CUI::CUI() :
    m_isFixed(true),
    m_isCursorOver()
{
	// �θ� Ŭ������ CObject�� �����ڰ� ���� ����ǹǷ�, �� �Լ����� �Ȱ��� Transform ������Ʈ�� �����Ͽ� RectTransformn ������Ʈ�� ������ �����.
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
}

CUI::~CUI()
{
}

CUI* CUI::Load(ifstream& in)
{
	CUI* ui = nullptr;
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<ClassType>")
		{
			int classType = 0;

			in.read(reinterpret_cast<char*>(&classType), sizeof(int));

			switch (classType)
			{
			case 0: ui = new CUI(); break;
			case 2: ui = new CMissionUI(); break;
			case 3: ui = new CKeyUI(); break;
			case 4: ui = new CHitUI(); break;
			}
		}
		else if (str == "<Name>")
		{
			File::ReadStringFromFile(in, ui->m_name);
		}
		else if (str == "<IsActive>")
		{
			in.read(reinterpret_cast<char*>(&ui->m_isActive), sizeof(int));
		}
		else if (str == "<RectTransform>")
		{
			// localPosition, localRotation, localScale, rect
			XMFLOAT3 transform[3] = {};
			XMFLOAT2 rect = {};

			in.read(reinterpret_cast<char*>(&transform[0]), 3 * sizeof(XMFLOAT3));
			in.read(reinterpret_cast<char*>(&rect), sizeof(XMFLOAT2));

			CRectTransform* rectTransform = static_cast<CRectTransform*>(ui->GetComponent(COMPONENT_TYPE::TRANSFORM));

			rectTransform->SetLocalPosition(transform[0]);
			rectTransform->SetLocalRotation(transform[1]);
			rectTransform->SetLocalScale(transform[2]);
			rectTransform->SetRect(rect);
			rectTransform->Update();
		}
		else if (str == "<Mesh>")
		{
			File::ReadStringFromFile(in, str);

			CMesh* mesh = CAssetManager::GetInstance()->GetMesh(str);

			ui->SetMesh(mesh);
		}
		else if (str == "<Materials>")
		{
			int materialCount = 0;

			in.read(reinterpret_cast<char*>(&materialCount), sizeof(int));

			if (materialCount > 0)
			{
				ui->m_materials.reserve(materialCount);

				// <Material>
				File::ReadStringFromFile(in, str);

				for (int i = 0; i < materialCount; ++i)
				{
					File::ReadStringFromFile(in, str);

					// ���͸��� �ν��Ͻ��� �����ϰ� �߰��Ѵ�.
					CMaterial* material = CAssetManager::GetInstance()->CreateMaterialInstance(str);

					ui->AddMaterial(material);
				}
			}
		}
		else if (str == "<SpriteSize>")
		{
			CSpriteRenderer* spriteRenderer = static_cast<CSpriteRenderer*>(ui->CreateComponent(COMPONENT_TYPE::SPRITE_RENDERER));
			XMINT2 spriteSize = {};

			in.read(reinterpret_cast<char*>(&spriteSize), sizeof(XMINT2));
			spriteRenderer->SetSpriteSize(spriteSize);
		}
		else if (str == "<FrameIndex>")
		{
			CSpriteRenderer* spriteRenderer = static_cast<CSpriteRenderer*>(ui->GetComponent(COMPONENT_TYPE::SPRITE_RENDERER));
			int frameIndex = 0;

			in.read(reinterpret_cast<char*>(&frameIndex), sizeof(int));
			spriteRenderer->SetFrameIndex(frameIndex);
		}
		else if (str == "<ChildCount>")
		{
			int childCount = 0;

			in.read(reinterpret_cast<char*>(&childCount), sizeof(int));

			for (int i = 0; i < childCount; ++i)
			{
				CUI* child = CUI::Load(in);

				if (child != nullptr)
				{
					ui->AddChild(child);
				}
			}
		}
		else if (str == "<Animator>")
		{
			CAnimator* animator = static_cast<CAnimator*>(ui->CreateComponent(COMPONENT_TYPE::ANIMATOR));

			animator->Load(in);
		}
		else if (str == "</Frame>")
		{
			ui->Init();
			break;
		}
	}

	return ui;
}

void CUI::SetFixed(bool isFixed)
{
    m_isFixed = isFixed;
}

bool CUI::IsFixed()
{
    return m_isFixed;
}

void CUI::CheckCursorOver()
{
	CRectTransform* rectTransform = static_cast<CRectTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	const XMFLOAT3& position = rectTransform->GetPosition();
	const XMFLOAT2& rect = rectTransform->GetRect();
    const XMFLOAT2& cursor = CURSOR;

    if ((position.x - 0.5f * rect.x <= cursor.x) && (cursor.x <= position.x + 0.5f * rect.x) &&
        (position.y - 0.5f * rect.y <= cursor.y) && (cursor.y <= position.y + 0.5f * rect.y))
    {
        m_isCursorOver = true;
    }
    else
    {
        m_isCursorOver = false;
    }
}

bool CUI::IsCursorOver()
{
    return m_isCursorOver;
}

CComponent* CUI::CreateComponent(COMPONENT_TYPE componentType)
{
	// �̹� �ش� ������Ʈ�� ������ �־��ٸ�, ���� �� ���� �����Ѵ�.
	if (m_components[static_cast<int>(componentType)] != nullptr)
	{
		delete m_components[static_cast<int>(componentType)];
		m_components[static_cast<int>(componentType)] = nullptr;
	}

	switch (componentType)
	{
		// UI�� CSkinningAnimator ������Ʈ�� �ƴ� CUIAnimator ������Ʈ�� ����Ѵ�.
	case COMPONENT_TYPE::ANIMATOR:
		m_components[static_cast<int>(componentType)] = new CUIAnimator();
		break;
	case COMPONENT_TYPE::TRANSFORM:
		// UI�� CTransform ������Ʈ�� �ƴ� CRectTransform ������Ʈ�� ����Ѵ�.
		m_components[static_cast<int>(componentType)] = new CRectTransform();
		break;
	default:
		// �� ���� ������Ʈ�� CObject�� �Լ��� ȣ���Ѵ�.
		return CObject::CreateComponent(componentType);
	}

	m_components[static_cast<int>(componentType)]->SetOwner(this);

	return m_components[static_cast<int>(componentType)];
}

void CUI::OnCursorOver()
{
}

void CUI::OnCursorLeftButtonDown()
{
}

void CUI::OnCursorLeftButtonUp()
{
}

void CUI::OnCursorLeftButtonClick()
{
}

void CUI::Update()
{
    CObject::Update();
    CheckCursorOver();
}

void CUI::Render(CCamera* camera)
{
	UpdateShaderVariables();

	CMesh* mesh = GetMesh();

	if (mesh != nullptr)
	{
		const vector<CMaterial*>& materials = GetMaterials();

		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
			materials[i]->UpdateShaderVariables();
			mesh->Render(i);
		}
	}
	
	const vector<CObject*>& children = GetChildren();

	for (const auto& child : children)
	{
		if (child->IsActive() && !child->IsDeleted())
		{
			child->Render(camera);
		}
	}
}

//=========================================================================================================================

CMissionUI::CMissionUI()
{
	CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
}

CMissionUI::~CMissionUI()
{
}

void CMissionUI::Init()
{
	CStateMachine* stateMachine = static_cast<CStateMachine*>(GetComponent(COMPONENT_TYPE::STATE_MACHINE));

	stateMachine->SetCurrentState(CMissionUIShowState::GetInstance());
}

//=========================================================================================================================

CKeyUI::CKeyUI()
{
	CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
}

CKeyUI::~CKeyUI()
{
}

//=========================================================================================================================

CHitUI::CHitUI()
{
	CreateComponent(COMPONENT_TYPE::STATE_MACHINE);
}

CHitUI::~CHitUI()
{
}

void CHitUI::Init()
{
	SetActive(false);
}
