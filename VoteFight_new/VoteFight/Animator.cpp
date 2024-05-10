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
#include "Player.h"

CAnimator::CAnimator() :
	m_isLoop(),
	m_isFinished(),
	m_animations(),
	m_playingAnimations(),
	m_frameIndices(),
	m_elapsedTime(),
	m_bBlending(true)
{
}

CAnimator::~CAnimator()
{
}

bool CAnimator::IsFinished()
{
	return m_isFinished;
}

void CAnimator::SetFrameIndex(int frameIndex, const string& key)
{
	if ((frameIndex < 0) || (frameIndex >= m_playingAnimations[key]->GetFrameCount()))
	{
		return;
	}

	m_frameIndices[key] = frameIndex;
}

int CAnimator::GetFrameIndex(const string& key)
{
	return m_frameIndices[key];
}

void CAnimator::SetWeight(const string& key, float fWeight)
{
	m_weights[key] = fWeight;
	//m_animations[key]->SetWeight(fWeight);
}

void CAnimator::Play(const string& key, bool isLoop, bool duplicatable)
{
	// 중복을 허용했다면, 동일 애니메이션으로 전이할 수 있다.
	if ((m_animations.find(key) == m_animations.end()) || ((!duplicatable) && (m_animations[key] == m_playingAnimations[key])))
	{
		return;
	}

	m_isLoop = isLoop;
	m_isFinished = false;
	m_bBlending = true;
	//memcpy(&m_playingAnimations[key], &m_animations[key], sizeof(CAnimation*));
	m_playingAnimations[key] = m_animations[key];
	m_upAnimation = key;
	m_frameIndices[key] = 0;
}

void CAnimator::BlendAnimation()
{
	if (m_bBlending)
	{
		// m_nUpState은 비중이 높아지고 있는 애니메이션의 인덱스이다. m_fStates[]는 각 애니메이션의 가중치이다.
		// 함수가 호출될 수록 m_nUpState에 해당하는 가중치는 0에서 1로 향하고, 나머지는 1에서 0으로 향한다.
		// 1에서 0으로 향하는 나머지들과 0에서 1로 향하는 모든 가중치는 합해서 1.0이 되어야 한다.

		float fUp = 1.0f - m_weights[m_upAnimation];		// 비중이 낮아지는 애니메이션의 가중치를 모두 합한 값
		float newUpWeight = m_weights[m_upAnimation] + 5.f * DT;

		// 비중이 완전히 기울면 블렌딩을 멈춘다.
		if (newUpWeight >= 1.0f)
		{
			auto it = m_playingAnimations.begin();
			while (it != m_playingAnimations.end()) {
				if (it->first != m_upAnimation) {
					m_weights[it->first] = 0.0f;
					// it->second->SetWeight(0.0f);
					it = m_playingAnimations.erase(it); 
				}
				else {
					++it; 
				}
			}
			m_bBlending = false;
			m_weights[m_upAnimation] = 1.0f;
			return;
		}

		// 비중이 완전히 기울지 않았으면 블렌딩을 한다.
		m_weights[m_upAnimation] = newUpWeight;
		for (auto selected : m_playingAnimations)
		{
			// 모든 가중치의 합은 1.0이 되어야 한다.
			// 그러므로 비중이 낮아지는 애니메이션의 감소 비율을 맞추어야 한다.
			if (selected.first == m_upAnimation) continue;
			float newWeight = (m_weights[selected.first] / fUp) * (1 - newUpWeight);
			m_weights[selected.first] = newWeight;
		}
	}
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

XMFLOAT3 QuaternionToEuler(XMFLOAT4 q) {
	XMFLOAT3 euler;

	// Roll (X 축을 중심으로 회전하는 각도)
	euler.x = XMConvertToDegrees(atan2(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y)));

	// Pitch (Y 축을 중심으로 회전하는 각도)
	euler.y = XMConvertToDegrees(asin(2 * (q.w * q.y - q.z * q.x)));

	// Yaw (Z 축을 중심으로 회전하는 각도)
	euler.z = XMConvertToDegrees(atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z)));

	return euler;
}

void CSkinningAnimator::Update()
{
	BlendAnimation();

	if ((m_isEnabled) && (!m_isFinished))
	{
		for (int i = 0; i < m_skinnedMeshCache.size(); ++i)
		{
			for (int j = 0; j < m_boneFrameCaches[i].size(); ++j)
			{
				CTransform* transform = static_cast<CTransform*>(m_boneFrameCaches[i][j]->GetComponent(COMPONENT_TYPE::TRANSFORM));
				transform->SetLocalPosition(XMFLOAT3(0, 0, 0));
				transform->SetLocalRotation(XMFLOAT3(0, 0, 0));
				transform->SetLocalScale(XMFLOAT3(0, 0, 0));
			}
		}

		for (auto selectedAnimation : m_playingAnimations)
		{
			if (selectedAnimation.second != nullptr)
			{
				m_elapsedTime += DT;

				float duration = 1.0f / selectedAnimation.second->GetFrameRate();

				while (m_elapsedTime >= duration)
				{
					// 축적된 시간이 애니메이션의 한 프레임 지속시간을 넘어서는 경우를 대비하여 0.0f으로 만드는 것이 아니라, 두 값의 차이로 설정한다.
					m_elapsedTime -= duration;

					++m_frameIndices[selectedAnimation.first];

					if (m_frameIndices[selectedAnimation.first] >= selectedAnimation.second->GetFrameCount())
					{
						if (m_isLoop)
						{
							m_frameIndices[selectedAnimation.first] = 0;
						}
						else
						{
							--m_frameIndices[selectedAnimation.first];
							m_isFinished = true;
							break;
						}
					}
				}

				// 이번 프레임의 애니메이션 변환 행렬을 각 뼈 프레임에 변환 행렬로 설정한다.
				CSkinningAnimation* playingAnimation = static_cast<CSkinningAnimation*>(selectedAnimation.second);
				const vector<vector<vector<XMFLOAT3>>>& bonePositions = playingAnimation->GetPositions();
				const vector<vector<vector<XMFLOAT3>>>& boneRotations = playingAnimation->GetRotations();
				const vector<vector<vector<XMFLOAT3>>>& boneScales = playingAnimation->GetScales();


				for (int i = 0; i < m_skinnedMeshCache.size(); ++i)
				{
					for (int j = 0; j < m_boneFrameCaches[i].size(); ++j)
					{
						CTransform* transform = static_cast<CTransform*>(m_boneFrameCaches[i][j]->GetComponent(COMPONENT_TYPE::TRANSFORM));

						transform->SetLocalPosition(Vector3::Add(transform->GetLocalPosition(), Vector3::ScalarProduct(bonePositions[i][j][m_frameIndices[selectedAnimation.first]], m_weights[selectedAnimation.first])));
						XMFLOAT3 Rotation = boneRotations[i][j][m_frameIndices[selectedAnimation.first]];
						
						// 상체 회전 (spine 기준)
						if (m_boneFrameCaches[i][j]->GetName() == static_cast<CPlayer*>(m_owner)->GetSpineName())
						{
							Rotation.y += static_cast<CPlayer*>(m_owner)->GetSpineAngle();
						}

						if (Rotation.x - transform->GetLocalRotation().x > 180) Rotation.x -= 360;
						if (Rotation.y - transform->GetLocalRotation().y > 180) Rotation.y -= 360;
						if (Rotation.z - transform->GetLocalRotation().z > 180) Rotation.z -= 360; 
						transform->SetLocalRotation(Vector3::Add(transform->GetLocalRotation(), Vector3::ScalarProduct(Rotation, m_weights[selectedAnimation.first])));
						transform->SetLocalScale(Vector3::Add(transform->GetLocalScale(), Vector3::ScalarProduct(boneScales[i][j][m_frameIndices[selectedAnimation.first]], m_weights[selectedAnimation.first])));
					}
				}
			}
		}
	}
}