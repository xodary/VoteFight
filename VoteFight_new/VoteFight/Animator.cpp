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
	// �ߺ��� ����ߴٸ�, ���� �ִϸ��̼����� ������ �� �ִ�.
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
		// m_nUpState�� ������ �������� �ִ� �ִϸ��̼��� �ε����̴�. m_fStates[]�� �� �ִϸ��̼��� ����ġ�̴�.
		// �Լ��� ȣ��� ���� m_nUpState�� �ش��ϴ� ����ġ�� 0���� 1�� ���ϰ�, �������� 1���� 0���� ���Ѵ�.
		// 1���� 0���� ���ϴ� ��������� 0���� 1�� ���ϴ� ��� ����ġ�� ���ؼ� 1.0�� �Ǿ�� �Ѵ�.

		float fUp = 1.0f - m_weights[m_upAnimation];		// ������ �������� �ִϸ��̼��� ����ġ�� ��� ���� ��
		float newUpWeight = m_weights[m_upAnimation] + 5.f * DT;

		// ������ ������ ���� ������ �����.
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

		// ������ ������ ����� �ʾ����� ������ �Ѵ�.
		m_weights[m_upAnimation] = newUpWeight;
		for (auto selected : m_playingAnimations)
		{
			// ��� ����ġ�� ���� 1.0�� �Ǿ�� �Ѵ�.
			// �׷��Ƿ� ������ �������� �ִϸ��̼��� ���� ������ ���߾�� �Ѵ�.
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

	// �����Ǵ� ��Ų �޽��� ���� �ִϸ��̼� ��Ʈ�ѷ��� �� ��ȯ ��� ���ҽ��� �������ش�.
	for (int i = 0; i < m_skinnedMeshCache.size(); ++i)
	{
		m_skinnedMeshCache[i]->SetBoneInfo(&m_boneFrameCaches[i], m_d3d12BoneTransformMatrixes[i], m_mappedBoneTransformMatrixes[i]);
	}
}

XMFLOAT3 QuaternionToEuler(XMFLOAT4 q) {
	XMFLOAT3 euler;

	// Roll (X ���� �߽����� ȸ���ϴ� ����)
	euler.x = XMConvertToDegrees(atan2(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y)));

	// Pitch (Y ���� �߽����� ȸ���ϴ� ����)
	euler.y = XMConvertToDegrees(asin(2 * (q.w * q.y - q.z * q.x)));

	// Yaw (Z ���� �߽����� ȸ���ϴ� ����)
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
					// ������ �ð��� �ִϸ��̼��� �� ������ ���ӽð��� �Ѿ�� ��츦 ����Ͽ� 0.0f���� ����� ���� �ƴ϶�, �� ���� ���̷� �����Ѵ�.
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

				// �̹� �������� �ִϸ��̼� ��ȯ ����� �� �� �����ӿ� ��ȯ ��ķ� �����Ѵ�.
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
						
						// ��ü ȸ�� (spine ����)
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