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
	// Player는 Root를 가지지 않음. 그래도 불렀다면 둘다 하고 싶은거임.
	if (m_owner->GetGroupType() == (int)GROUP_TYPE::PLAYER && type == ANIMATION_BONE::ROOT) {
		Play(key, isLoop, UPPER, duplicatable);
		Play(key, isLoop, LOWER, duplicatable);
		return;
	}
	
	// 중복을 허용했다면, 동일 애니메이션으로 전이할 수 있다.
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
				if (m_skinnedMeshCache[i] == nullptr) cout << str << " : Skinned Mesh를 찾을 수 없음." << endl;

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

void CAnimator::BlendAnimation()
{
	for (auto& animation : m_animationMask)
	{
		if (!animation.second.m_bBlending) continue;

		string upAni = animation.second.m_upAnimation;

		float fUp = 1.0f - animation.second.m_weights[upAni];		// 비중이 낮아지는 애니메이션의 가중치를 모두 합한 값
		float newUpWeight = animation.second.m_weights[upAni] + 5.f * DT;

		// 비중이 완전히 기울면 블렌딩을 멈춘다.
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

		// 비중이 완전히 기울지 않았으면 블렌딩을 한다.
		animation.second.m_weights[upAni] = newUpWeight;
		for (auto selected : animation.second.m_playingAnimations)
		{
			// 모든 가중치의 합은 1.0이 되어야 한다.
			// 그러므로 비중이 낮아지는 애니메이션의 감소 비율을 맞추어야 한다.
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

			// 이번 프레임의 애니메이션 변환 행렬을 각 뼈 프레임에 변환 행렬로 설정한다.
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