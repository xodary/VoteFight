#pragma once
#include "Component.h"

class CSkinnedMesh;
class CAnimation;

enum ANIMATION_BONE { ROOT, UPPER, LOWER };

class CAnimator abstract : public CComponent
{
public:

	class AnimationMasking {
	public:
		ANIMATION_BONE						type;
		bool								m_bBlending;
		unordered_map<string, CAnimation*>	m_playingAnimations;
		unordered_map<string, float>		m_weights;
		unordered_map<string, int>			m_frameIndices;
		string								m_upAnimation;
		vector<CObject*>					m_animationBones;
		vector<CObject*>					m_maskBones;
		bool								m_isLoop;
		unordered_map<string, bool>			m_isFinished;
		unordered_map<string, float>		m_elapsedTime;
	};

	unordered_map<string, float>						m_speed;
	bool												m_isAnimationMasked;
	unordered_map<ANIMATION_BONE, AnimationMasking>		m_animationMask;
	unordered_map<string, CAnimation*>					m_animations;

	CAnimator();
	~CAnimator();
	
	bool IsFinished(const string& key, const ANIMATION_BONE& type);

	void SetWeight(const string& key, const ANIMATION_BONE& type, float fWeight);
	void SetAnimateBone(CObject* bone, const ANIMATION_BONE& type);
	void SetMaskBone(CObject* bone, const ANIMATION_BONE& type);
	int GetFrameIndex(const string& key, const ANIMATION_BONE& type);
	void Play(const string& key, bool isLoop, const ANIMATION_BONE& type=ROOT, bool duplicatable = false);
	void BlendAnimation();
	void SetBlending(const ANIMATION_BONE& type, bool b) { m_animationMask[type].m_bBlending = b; }
	void SetSpeed(const string& key, float speed) { m_speed[key] = speed; }
	virtual void Load(ifstream& in) = 0;
	virtual void Update() = 0;
};

//=========================================================================================================================

class CSkinningAnimator : public CAnimator
{
private:
	vector<CSkinnedMesh*>		   m_skinnedMeshCache;
	vector<vector<CObject*>>       m_boneFrameCaches; // [skinnedMesh][boneFrame]

	vector<ComPtr<ID3D12Resource>> m_d3d12BoneTransformMatrixes;
	vector<XMFLOAT4X4*>			   m_mappedBoneTransformMatrixes;

public:
	CSkinningAnimator();
	virtual ~CSkinningAnimator();

	virtual void Load(ifstream& in);

	virtual void UpdateShaderVariables();

	virtual void Update();
};