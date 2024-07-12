#include "pch.h"
#include "UI.h"
#include "CameraManager.h"
#include "GameFramework.h"
#include "Camera.h"
#include "AssetManager.h"
#include "InputManager.h"

#include "Mesh.h"
#include "Texture.h"
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
	// 부모 클래스인 CObject의 생성자가 먼저 실행되므로, 이 함수에서 똑같이 Transform 컴포넌트를 생성하여 RectTransformn 컴포넌트를 갖도록 만든다.
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
			
			RECT winrect;
			GetClientRect(CGameFramework::GetInstance()->GetHwnd(), &winrect);

			transform[0].x *= (winrect.right - winrect.left)/2;
			transform[0].y *= (winrect.bottom - winrect.top)/2;
			// rectTransform->SetLocalPosition(transform[0]);
			// rectTransform->SetLocalRotation(transform[1]);
			// rectTransform->SetLocalScale(transform[2]);
			rectTransform->SetPosition(transform[0]);
			rectTransform->SetRotation(transform[1]);
			rectTransform->SetScale(transform[2]);

			rect.x *= (winrect.right - winrect.left) / 2;
			rect.y *= (winrect.bottom - winrect.top) / 2;
			rectTransform->SetRect(rect);
			rectTransform->Update();
		}
		else if (str == "<Mesh>")
		{
			File::ReadStringFromFile(in, str);

			CRectMesh* mesh = new CRectMesh();

			ui->SetMesh(mesh);
		}
		else if (str == "<Textures>")
		{
			int materialCount = 0;

			in.read(reinterpret_cast<char*>(&materialCount), sizeof(int));

			if (materialCount > 0)
			{
				ui->m_materials.reserve(materialCount);

				// <Texture>
				File::ReadStringFromFile(in, str);

				for (int i = 0; i < materialCount; ++i)
				{
					File::ReadStringFromFile(in, str);

					CMaterial* material = CAssetManager::GetInstance()->CreateMaterial(str);
					CTexture* texture = CAssetManager::GetInstance()->CreateTexture(str, str, TEXTURE_TYPE::ALBEDO_MAP);
					material->SetTexture(texture);

					CShader* shader = CAssetManager::GetInstance()->GetShader("UI");
					material->AddShader(shader);
					material->SetStateNum(1);

					ui->AddMaterial(material);
				}
			}
		}
		else if (str == "<SpriteSize>")
		{
			CSpriteRenderer* spriteRenderer = static_cast<CSpriteRenderer*>(ui->CreateComponent(COMPONENT_TYPE::SPRITE_RENDERER));
			XMFLOAT2 spriteSize = {};

			in.read(reinterpret_cast<char*>(&spriteSize), sizeof(XMFLOAT2));
			spriteRenderer->SetSpriteSize(spriteSize);
		}
		else if (str == "<FrameIndex>")
		{
			CSpriteRenderer* spriteRenderer = static_cast<CSpriteRenderer*>(ui->GetComponent(COMPONENT_TYPE::SPRITE_RENDERER));
			XMFLOAT2 frameIndex = {};

			in.read(reinterpret_cast<char*>(&frameIndex), sizeof(XMFLOAT2));
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
	// 이미 해당 컴포넌트를 가지고 있었다면, 삭제 후 새로 생성한다.
	if (m_components[static_cast<int>(componentType)] != nullptr)
	{
		delete m_components[static_cast<int>(componentType)];
		m_components[static_cast<int>(componentType)] = nullptr;
	}

	switch (componentType)
	{
		// UI는 CSkinningAnimator 컴포넌트가 아닌 CUIAnimator 컴포넌트를 사용한다.
	//case COMPONENT_TYPE::ANIMATOR:
	//	m_components[static_cast<int>(componentType)] = new CUIAnimator();
	//	break;
	case COMPONENT_TYPE::TRANSFORM:
		// UI는 CTransform 컴포넌트가 아닌 CRectTransform 컴포넌트를 사용한다.
		m_components[static_cast<int>(componentType)] = new CRectTransform();
		break;
	case COMPONENT_TYPE::SPRITE_RENDERER:
		m_components[static_cast<int>(componentType)] = new CSpriteRenderer();
		break;
	default:
		// 그 외의 컴포넌트는 CObject의 함수를 호출한다.
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

	CRectMesh* mesh = static_cast<CRectMesh*>(GetMesh());

	if (mesh != nullptr)
	{
		const vector<CMaterial*>& materials = GetMaterials();

		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
			materials[i]->UpdateShaderVariables();
			mesh->Render();
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

CBilboardUI::CBilboardUI()
{
}

CBilboardUI::~CBilboardUI()
{
}

CComponent* CBilboardUI::CreateComponent(COMPONENT_TYPE componentType)
{
	// 이미 해당 컴포넌트를 가지고 있었다면, 삭제 후 새로 생성한다.
	if (m_components[static_cast<int>(componentType)] != nullptr)
	{
		delete m_components[static_cast<int>(componentType)];
		m_components[static_cast<int>(componentType)] = nullptr;
	}

	switch (componentType)
	{
		// UI는 CSkinningAnimator 컴포넌트가 아닌 CUIAnimator 컴포넌트를 사용한다.
	//case COMPONENT_TYPE::ANIMATOR:
	//	m_components[static_cast<int>(componentType)] = new CUIAnimator();
	//	break;
	case COMPONENT_TYPE::TRANSFORM:
		// UI는 CTransform 컴포넌트가 아닌 CRectTransform 컴포넌트를 사용한다.
		m_components[static_cast<int>(componentType)] = new CTransform();
		break;
	default:
		// 그 외의 컴포넌트는 CObject의 함수를 호출한다.
		return CObject::CreateComponent(componentType);
	}

	m_components[static_cast<int>(componentType)]->SetOwner(this);

	return m_components[static_cast<int>(componentType)];
}

CSpeechBubbleUI::CSpeechBubbleUI(CObject* owner)
{
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
	m_owner = owner;
	m_isActive = true;
	CMaterial* material = new CMaterial();
	material->SetStateNum(0);
	CRectMesh* mesh = new CRectMesh(2, 1);
	SetMesh(mesh);
	material->SetTexture(CAssetManager::GetInstance()->GetTexture("speech_bubble"));
	CShader* BilboardShader = CAssetManager::GetInstance()->GetShader("Bilboard");
	material->m_shaders.push_back(BilboardShader);
	m_materials.push_back(material);
}

void CSpeechBubbleUI::Update()
{
	CTransform* transform = reinterpret_cast<CTransform*>(m_owner->GetComponent(COMPONENT_TYPE::TRANSFORM));
	CTransform* uitransform = reinterpret_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	CTransform* cameraTransform = reinterpret_cast<CTransform*>(CCameraManager::GetInstance()->GetMainCamera()->GetComponent(COMPONENT_TYPE::TRANSFORM));
	uitransform->LookTo(Vector3::Subtract(uitransform->GetPosition(), cameraTransform->GetPosition()));

	uitransform->SetPosition(XMFLOAT3(transform->GetPosition().x, transform->GetPosition().y + 6.0f, transform->GetPosition().z));

	CObject::Update();
}

CHPbarUI::CHPbarUI(CObject* owner)
{
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
	m_owner = owner;
	m_isActive = true;
	CMaterial* material = new CMaterial();
	material->SetStateNum(0);
	material->SetColor(XMFLOAT4(1, 0, 0, 1));
	CRectMesh* mesh = new CRectMesh(1.5, 0.1);
	SetMesh(mesh);
	CShader* BilboardShader = CAssetManager::GetInstance()->GetShader("Bilboard");
	material->m_shaders.push_back(BilboardShader);
	m_materials.push_back(material);
}

void CHPbarUI::Update()
{
	CTransform* transform = reinterpret_cast<CTransform*>(m_owner->GetComponent(COMPONENT_TYPE::TRANSFORM));
	CTransform* uitransform = reinterpret_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));

	CTransform* cameraTransform = reinterpret_cast<CTransform*>(CCameraManager::GetInstance()->GetMainCamera()->GetComponent(COMPONENT_TYPE::TRANSFORM));
	uitransform->LookTo(Vector3::Subtract(uitransform->GetPosition(), cameraTransform->GetPosition()));

	uitransform->SetPosition(XMFLOAT3(transform->GetPosition().x, transform->GetPosition().y + 3.5f, transform->GetPosition().z));
	CObject::Update();
}

CTextUI::CTextUI(CObject* owner)
{
	CreateComponent(COMPONENT_TYPE::TRANSFORM);
	m_owner = owner;
	m_isActive = true;
	SetName("Namebar");
	CMaterial* textMaterial = new CMaterial();
	textMaterial->SetStateNum(1);
	CTextMesh* mesh = new CTextMesh(CGameFramework::GetInstance()->m_FontData, owner->m_name.c_str(), 0, 0, 0.1f, 0.3f);
	SetMesh(mesh);
	textMaterial->SetTexture(CAssetManager::GetInstance()->GetTexture("text"));
	CShader* BilboardShader = CAssetManager::GetInstance()->GetShader("Bilboard");
	textMaterial->m_shaders.push_back(BilboardShader);
	m_materials.push_back(textMaterial);
}

void CTextUI::Render(CCamera* camera)
{
	UpdateShaderVariables();

	CTextMesh* mesh = static_cast<CTextMesh*>(GetMesh());

	if (mesh != nullptr)
	{
		const vector<CMaterial*>& materials = GetMaterials();

		for (int i = 0; i < materials.size(); ++i)
		{
			materials[i]->SetPipelineState(RENDER_TYPE::STANDARD);
			materials[i]->UpdateShaderVariables();
			mesh->Render();
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

void CTextUI::Update()
{
	CTransform* transform = reinterpret_cast<CTransform*>(m_owner->GetComponent(COMPONENT_TYPE::TRANSFORM));
	CTransform* uitransform = reinterpret_cast<CTransform*>(GetComponent(COMPONENT_TYPE::TRANSFORM));
	
	CTransform* cameraTransform = reinterpret_cast<CTransform*>(CCameraManager::GetInstance()->GetMainCamera()->GetComponent(COMPONENT_TYPE::TRANSFORM));
	uitransform->LookTo(Vector3::Subtract(uitransform->GetPosition(), cameraTransform->GetPosition()));
	
	uitransform->SetPosition(XMFLOAT3(transform->GetPosition().x, transform->GetPosition().y + 4.2f, transform->GetPosition().z));

	CObject::Update();
}

