#pragma once
#include "Component.h"

class CSkinnedMesh;
class CAnimation;

class CAnimator abstract : public CComponent
{
protected:
	bool							   m_isLoop;
	bool							   m_isFinished;
	bool							   m_bBlending;
	unordered_map<string, CAnimation*> m_animations;
	unordered_map<string, CAnimation*> m_playingAnimations;
	// unordered_map<string, float>	   m_weights;
	unordered_map<string, int>	       m_frameIndices;
	float							   m_elapsedTime;
	string							   m_upAnimation;

public:
	CAnimator();
	~CAnimator();
	
	bool IsFinished();

	void SetFrameIndex(int frameIndex, const string& key);
	int GetFrameIndex(const string& key);
	void SetWeight(const string& key, float fWeight);
	void Play(const string& key, bool isLoop, bool duplicatable = false);
	void BlendAnimation();

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