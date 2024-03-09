#pragma once
#include "Component.h"

class CSkinnedMesh;
class CAnimation;

class CAnimator abstract : public CComponent
{
protected:
	bool							   m_isLoop;
	bool							   m_isFinished;

	unordered_map<string, CAnimation*> m_animations;
	CAnimation*						   m_playingAnimation;
	int							       m_frameIndex;
	float							   m_elapsedTime;

public:
	CAnimator();
	~CAnimator();
	
	bool IsFinished();

	void SetFrameIndex(int frameIndex);
	int GetFrameIndex();

	void Play(const string& key, bool isLoop, bool duplicatable = false);

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

//=========================================================================================================================

class CUIAnimator : public CAnimator
{
private:
	vector<CObject*> m_uiFrameCache;

public:
	CUIAnimator();
	virtual ~CUIAnimator();

	virtual void Load(ifstream& in);

	virtual void Update();
};
