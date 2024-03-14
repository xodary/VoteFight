#include "pch.h"
#include "Animator.h"

#include "GameFramework.h"

#include "TimeManager.h"
#include "AssetManager.h"

#include "Object.h"

#include "SkinnedMesh.h"
#include "Material.h"
#include "Animation.h"

#include "Transform.h"

CAnimator::CAnimator() :
	m_isLoop(),
	m_isFinished(),
	m_animations(),
	m_playingAnimation(),
	m_frameIndex(),
	m_elapsedTime()
{
}

CAnimator::~CAnimator()
{
}

bool CAnimator::IsFinished()
{
	return m_isFinished;
}

void CAnimator::SetFrameIndex(int frameIndex)
{
	if ((frameIndex < 0) || (frameIndex >= m_playingAnimation->GetFrameCount()))
	{
		return;
	}

	m_frameIndex = frameIndex;
}

int CAnimator::GetFrameIndex()
{
	return m_frameIndex;
}

void CAnimator::Play(const string& key, bool isLoop, bool duplicatable)
{
	// 중복을 허용했다면, 동일 애니메이션으로 전이할 수 있다.
	if ((m_animations.find(key) == m_animations.end()) || ((!duplicatable) && (m_animations[key] == m_playingAnimation)))
	{
		return;
	}

	m_isLoop = isLoop;
	m_isFinished = false;
	m_playingAnimation = m_animations[key];
	m_frameIndex = 0;
}

//=========================================================================================================================

CSkinningAnimator::CSkinningAnimator() :
	m_skinnedMeshCache(),
	m_boneFrameCaches(),
	m_d3d12BoneTransformMatrixes(),
	m_mappedBoneTransformMatrixes()
{
}

CSkinningAnimator::~CSkinningAnimator()
{
}

void CSkinningAnimator::Load(ifstream& in)
{
	ID3D12Device* d3d12Device = CGameFramework::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	string str;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<FileName>")
		{
			File::ReadStringFromFile(in, str);
			CAssetManager::GetInstance()->LoadSkinningAnimations(str);

			const vector<CAnimation*>& animations = CAssetManager::GetInstance()->GetAnimations(str);

			for (const auto& animation : animations)
			{
				m_animations.emplace(animation->GetName(), animation);
			}
		}
		else if (str == "<SkinnedMeshes>")
		{
			int skinnedMeshCount = 0;

			in.read(reinterpret_cast<char*>(&skinnedMeshCount), sizeof(int));
			m_skinnedMeshCache.resize(skinnedMeshCount);
			m_boneFrameCaches.resize(skinnedMeshCount);

			unordered_map<string, CObject*> boneFrameCache;

			for (int i = 0; i < skinnedMeshCount; ++i)
			{
				// <Name>
				File::ReadStringFromFile(in, str);

				File::ReadStringFromFile(in, str);
				m_skinnedMeshCache[i] = static_cast<CSkinnedMesh*>(CAssetManager::GetInstance()->GetMesh(str));

				// <Bones>
				File::ReadStringFromFile(in, str);

				int boneCount = 0;

				in.read(reinterpret_cast<char*>(&boneCount), sizeof(int));
				m_boneFrameCaches[i].resize(boneCount);

				for (int j = 0; j < boneCount; ++j)
				{
					File::ReadStringFromFile(in, str);

					if (boneFrameCache.find(str) == boneFrameCache.end())
					{
						boneFrameCache.emplace(str, m_owner->FindFrame(str));
					}

					m_boneFrameCaches[i][j] = boneFrameCache[str];
				}
			}

			m_d3d12BoneTransformMatrixes.resize(skinnedMeshCount);
			m_mappedBoneTransformMatrixes.resize(skinnedMeshCount);

			int bytes = ((sizeof(XMFLOAT4X4) * MAX_BONES) + 255) & ~255;

			for (int i = 0; i < skinnedMeshCount; ++i)
			{
				m_d3d12BoneTransformMatrixes[i] = DX::CreateBufferResource(d3d12Device, d3d12GraphicsCommandList, nullptr, bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);
				m_d3d12BoneTransformMatrixes[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedBoneTransformMatrixes[i]));
			}
		}
		else if (str == "</Animator>")
		{
			break;
		}
	}
}

void CSkinningAnimator::UpdateShaderVariables()
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();

	// 공유되는 스킨 메쉬에 현재 애니메이션 컨트롤러의 뼈 변환 행렬 리소스를 설정해준다.
	for (int i = 0; i < m_skinnedMeshCache.size(); ++i)
	{
		m_skinnedMeshCache[i]->SetBoneInfo(&m_boneFrameCaches[i], m_d3d12BoneTransformMatrixes[i], m_mappedBoneTransformMatrixes[i]);
	}
}

void CSkinningAnimator::Update()
{
	if ((m_isEnabled) && (!m_isFinished))
	{
		if (m_playingAnimation != nullptr)
		{
			m_elapsedTime += DT;

			float duration = 1.0f / m_playingAnimation->GetFrameRate();

			while (m_elapsedTime >= duration)
			{
				// 축적된 시간이 애니메이션의 한 프레임 지속시간을 넘어서는 경우를 대비하여 0.0f으로 만드는 것이 아니라, 두 값의 차이로 설정한다.
				m_elapsedTime -= duration;
				++m_frameIndex;

				if (m_frameIndex >= m_playingAnimation->GetFrameCount())
				{
					if (m_isLoop)
					{
						m_frameIndex = 0;
					}
					else
					{
						--m_frameIndex;
						m_isFinished = true;
						break;
					}
				}
			}

			// 이번 프레임의 애니메이션 변환 행렬을 각 뼈 프레임에 변환 행렬로 설정한다.
			CSkinningAnimation* playingAnimation = static_cast<CSkinningAnimation*>(m_playingAnimation);
			const vector<vector<vector<XMFLOAT3>>>& bonePositions = playingAnimation->GetPositions();
			const vector<vector<vector<XMFLOAT3>>>& boneRotations = playingAnimation->GetRotations();
			const vector<vector<vector<XMFLOAT3>>>& boneScales = playingAnimation->GetScales();

			for (int i = 0; i < m_skinnedMeshCache.size(); ++i)
			{
				for (int j = 0; j < m_boneFrameCaches[i].size(); ++j)
				{
					CTransform* transform = static_cast<CTransform*>(m_boneFrameCaches[i][j]->GetComponent(COMPONENT_TYPE::TRANSFORM));

					transform->SetLocalPosition(bonePositions[i][j][m_frameIndex]);
					transform->SetLocalRotation(boneRotations[i][j][m_frameIndex]);
					transform->SetLocalScale(boneScales[i][j][m_frameIndex]);
				}
			}
		}
	}
}

//=========================================================================================================================

CUIAnimator::CUIAnimator() :
	m_uiFrameCache()
{
}

CUIAnimator::~CUIAnimator()
{
}

void CUIAnimator::Load(ifstream& in)
{
	ID3D12GraphicsCommandList* d3d12GraphicsCommandList = CGameFramework::GetInstance()->GetGraphicsCommandList();
	CAssetManager::GetInstance()->LoadUIAnimations(in, m_owner->GetName());
	const vector<CAnimation*>& animations = CAssetManager::GetInstance()->GetAnimations(m_owner->GetName());

	for (const auto& animation : animations)
	{
		m_animations.emplace(animation->GetName(), animation);
	}

	// 스택을 사용하여 깊이 우선 탐색(DFS)으로 owner의 모든 객체를 순회하며 저장한다.
	stack<CObject*> st;

	st.push(m_owner);

	while (!st.empty())
	{
		CObject* ui = st.top();

		st.pop();
		m_uiFrameCache.push_back(ui);

		const vector<CObject*>& children = ui->GetChildren();

		for (int i = static_cast<int>(children.size() - 1); i >= 0; --i)
		{
			st.push(children[i]);
		}
	}
}

void CUIAnimator::Update()
{
	if (m_isEnabled && !m_isFinished)
	{
		if (m_playingAnimation != nullptr)
		{
			m_elapsedTime += DT;

			float duration = 1.0f / m_playingAnimation->GetFrameRate();

			while (m_elapsedTime >= duration)
			{
				// 축적된 시간이 애니메이션의 한 프레임 지속시간을 넘어서는 경우를 대비하여 0.0f으로 만드는 것이 아니라, 두 값의 차이로 설정한다.
				m_elapsedTime -= duration;
				++m_frameIndex;

				if (m_frameIndex >= m_playingAnimation->GetFrameCount())
				{
					if (m_isLoop)
					{
						m_frameIndex = 0;
					}
					else
					{
						--m_frameIndex;
						m_isFinished = true;
						break;
					}
				}
			}

			// 이번 프레임의 애니메이션 변환 행렬을 각 뼈 프레임에 변환 행렬로 설정한다.
			CUIAnimation* playingAnimation = static_cast<CUIAnimation*>(m_playingAnimation);
			const vector<vector<XMFLOAT3>>& uiPositions = playingAnimation->GetPositions();
			const vector<vector<XMFLOAT3>>& uiRotations = playingAnimation->GetRotations();
			const vector<vector<XMFLOAT3>>& uiScales = playingAnimation->GetScales();
			const vector<vector<XMFLOAT4>>& uiColors = playingAnimation->GetColors();

			for (int i = 0, j = 0; i < m_uiFrameCache.size(); ++i)
			{
				CRectTransform* transform = static_cast<CRectTransform*>(m_uiFrameCache[i]->GetComponent(COMPONENT_TYPE::TRANSFORM));

				transform->SetLocalPosition(uiPositions[m_frameIndex][i]);
				transform->SetLocalRotation(uiRotations[m_frameIndex][i]);
				transform->SetLocalScale(uiScales[m_frameIndex][i]);

				const vector<CMaterial*>& materials = m_uiFrameCache[i]->GetMaterials();

				if (!materials.empty())
				{
					materials[0]->SetColor(uiColors[m_frameIndex][j++]);
				}
			}
		}
	}
}
