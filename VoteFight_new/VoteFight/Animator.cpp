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
	m_animations(),
	m_isAnimationMasked(false),
	m_animationMask()
{
}

CAnimator::~CAnimator()
{
}

bool CAnimator::IsFinished(const string& key, const ANIMATION_BONE& type)
{
	if (m_animationMask[type].m_playingAnimations.find(key) == m_animationMask[type].m_playingAnimations.end())
		return true;
	return m_animationMask[type].m_isFinished[key];
}

void CAnimator::SetWeight(const string& key, const ANIMATION_BONE& type, float fWeight)
{
	m_animationMask[type].m_weights[key] = fWeight;
}

void CAnimator::SetAnimateBone(CObject* bone, const ANIMATION_BONE& type)
{
	m_animationMask[type].m_animationBones.push_back(bone);
}

void CAnimator::SetMaskBone(CObject* bone, const ANIMATION_BONE& type)
{
	m_animationMask[type].m_maskBones.push_back(bone);
}

int CAnimator::GetFrameIndex(const string& key, const ANIMATION_BONE& type)
{
	return m_animationMask[type].m_frameIndices[key];
}

void CAnimator::Play(const string& key, bool isLoop, const ANIMATION_BONE& type, bool duplicatable)
{
	// Player�� Root�� ������ ����. �׷��� �ҷ��ٸ� �Ѵ� �ϰ� ��������.
	if (m_owner->GetGroupType() == (int)GROUP_TYPE::PLAYER && type == ANIMATION_BONE::ROOT) {
		Play(key, isLoop, UPPER, duplicatable);
		Play(key, isLoop, LOWER, duplicatable);
		return;
	}
	
	// �ߺ��� ����ߴٸ�, ���� �ִϸ��̼����� ������ �� �ִ�.
	if ((m_animations.find(key) == m_animations.end()) || ((!duplicatable) && (m_animations[key] == m_animationMask[type].m_playingAnimations[key])))
	{
		return;
	}

	m_animationMask[type].m_isLoop = isLoop;
	m_animationMask[type].m_isFinished[key] = false;
	m_animationMask[type].m_bBlending = true;
	//memcpy(&m_playingAnimations[key], &m_animations[key], sizeof(CAnimation*));
	m_animationMask[type].m_playingAnimations[key] = m_animations[key];
	m_speed[key] = 1.0f;
	m_animationMask[type].m_upAnimation = key;
	m_animationMask[type].m_frameIndices[key] = 0;
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
				if (m_skinnedMeshCache[i] == nullptr) cout << str << " : Skinned Mesh�� ã�� �� ����." << endl;

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

void CAnimator::BlendAnimation()
{
	for (auto& animation : m_animationMask)
	{
		if (!animation.second.m_bBlending) continue;

		string upAni = animation.second.m_upAnimation;

		float fUp = 1.0f - animation.second.m_weights[upAni];		// ������ �������� �ִϸ��̼��� ����ġ�� ��� ���� ��
		float newUpWeight = animation.second.m_weights[upAni] + 5.f * DT;

		// ������ ������ ���� ������ �����.
		if (newUpWeight >= 1.0f)
		{
			auto it = animation.second.m_playingAnimations.begin();
			while (it != animation.second.m_playingAnimations.end()) {
				if (it->first != upAni) {
					animation.second.m_weights[it->first] = 0.0f;
					it = animation.second.m_playingAnimations.erase(it);
				}
				else {
					++it;
				}
			}
			animation.second.m_bBlending = false;
			animation.second.m_weights[upAni] = 1.0f;
			continue;
		}

		// ������ ������ ����� �ʾ����� ������ �Ѵ�.
		animation.second.m_weights[upAni] = newUpWeight;
		for (auto selected : animation.second.m_playingAnimations)
		{
			// ��� ����ġ�� ���� 1.0�� �Ǿ�� �Ѵ�.
			// �׷��Ƿ� ������ �������� �ִϸ��̼��� ���� ������ ���߾�� �Ѵ�.
			if (selected.first == upAni) continue;
			float newWeight = (animation.second.m_weights[selected.first] / fUp) * (1 - newUpWeight);
			animation.second.m_weights[selected.first] = newWeight;
		}
	}
}

void CSkinningAnimator::Update()
{
	BlendAnimation();

	for (int i = 0; i < m_skinnedMeshCache.size(); ++i) {
		for (int j = 0; j < m_boneFrameCaches[i].size(); ++j) {
			CTransform* transform = static_cast<CTransform*>(m_boneFrameCaches[i][j]->GetComponent(COMPONENT_TYPE::TRANSFORM));
			transform->SetLocalPosition(XMFLOAT3(0, 0, 0));
			transform->SetLocalRotation(XMFLOAT3(0, 0, 0));
			transform->SetLocalScale(XMFLOAT3(0, 0, 0));
		}
	}

	for (auto& animation : m_animationMask)
	{
		for (auto& selectedAnimation : animation.second.m_playingAnimations)
		{
			if (selectedAnimation.second == nullptr) continue;

			animation.second.m_elapsedTime[selectedAnimation.first] += DT;
			float duration = 1.0f / selectedAnimation.second->GetFrameRate() / m_speed[selectedAnimation.first];

			while (animation.second.m_elapsedTime[selectedAnimation.first] >= duration)
			{
				animation.second.m_elapsedTime[selectedAnimation.first] -= duration;

				++animation.second.m_frameIndices[selectedAnimation.first];

				if (animation.second.m_frameIndices[selectedAnimation.first] >= selectedAnimation.second->GetFrameCount())
				{
					if (animation.second.m_isLoop)
					{
						animation.second.m_frameIndices[selectedAnimation.first] = 0;
					}
					else
					{
						--animation.second.m_frameIndices[selectedAnimation.first];
						animation.second.m_isFinished[selectedAnimation.first] = true;
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
					bool maskBone = false;
					for (auto m : animation.second.m_maskBones) {
						if (m->isChild(m_boneFrameCaches[i][j]->GetName())) maskBone = true;
					}
					if (maskBone) continue;
					bool aniBone = false;
					for (auto n : animation.second.m_animationBones) {
						if (n->isChild(m_boneFrameCaches[i][j]->GetName())) aniBone = true;
					}
					if (!aniBone) continue;
					CTransform* transform = static_cast<CTransform*>(m_boneFrameCaches[i][j]->GetComponent(COMPONENT_TYPE::TRANSFORM));
					XMFLOAT3 Postion = bonePositions[i][j][animation.second.m_frameIndices[selectedAnimation.first]];
					XMFLOAT3 Rotation = boneRotations[i][j][animation.second.m_frameIndices[selectedAnimation.first]];
					XMFLOAT3 Scale = boneScales[i][j][animation.second.m_frameIndices[selectedAnimation.first]];

					if (Rotation.x - transform->GetLocalRotation().x > 180) Rotation.x -= 360;
					if (Rotation.y - transform->GetLocalRotation().y > 180) Rotation.y -= 360;
					if (Rotation.z - transform->GetLocalRotation().z > 180) Rotation.z -= 360;

					float weight = animation.second.m_weights[selectedAnimation.first];
					transform->SetLocalPosition(Vector3::Add(transform->GetLocalPosition(), Vector3::ScalarProduct(Postion, weight)));
					transform->SetLocalRotation(Vector3::Add(transform->GetLocalRotation(), Vector3::ScalarProduct(Rotation, weight)));
					transform->SetLocalScale(Vector3::Add(transform->GetLocalScale(), Vector3::ScalarProduct(Scale, weight)));
				}
			}
		}
	}
}