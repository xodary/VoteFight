#pragma once
#include "Asset.h"

class CAnimation abstract : public CAsset
{
	friend class CAssetManager;

protected:
	int	  m_frameRate;
	int	  m_frameCount;
	float m_duration;

protected:
	// �� ��ü�� ������ ������ CAssetManager�� ���ؼ��� �Ͼ��.
	// �̶�, �ڽ� Ŭ�������� �� Ŭ������ �����ڸ� ȣ���ؾ� �ϹǷ�, ���������ڸ� proteced�� �����Ͽ���.
	CAnimation();

public:
	// �Ҹ����� ��쿡�� SafeDelete �ܺ� �Լ��� �̿��ϱ� ������ ���� �����ڸ� public���� �����Ͽ���.
	virtual ~CAnimation();

	int GetFrameRate();
	int GetFrameCount();
	float GetDuration();

	virtual void Load(ifstream& in) = 0;
};

//=========================================================================================================================

class CSkinningAnimation : public CAnimation
{
private:
	vector<vector<vector<XMFLOAT3>>> m_bonePositions; // [skinnedMesh][bone][frameIndex]
	vector<vector<vector<XMFLOAT3>>> m_boneRotations; // [skinnedMesh][bone][frameIndex]
	vector<vector<vector<XMFLOAT3>>> m_boneScales;    // [skinnedMesh][bone][frameIndex]

public:
	CSkinningAnimation();
	virtual ~CSkinningAnimation();

	const vector<vector<vector<XMFLOAT3>>>& GetPositions();
	const vector<vector<vector<XMFLOAT3>>>& GetRotations();
	const vector<vector<vector<XMFLOAT3>>>& GetScales();

	virtual void Load(ifstream& in);
};

//=========================================================================================================================

class CUIAnimation : public CAnimation
{
private:
	vector<vector<XMFLOAT3>> m_uiPositions;      // [frameIndex][all ui]
	vector<vector<XMFLOAT3>> m_uiRotations;      // [frameIndex][all ui]
	vector<vector<XMFLOAT3>> m_uiScales;         // [frameIndex][all ui]
	vector<vector<XMFLOAT4>> m_uiMaterialColors; // [frameIndex][has Material ui]

public:
	CUIAnimation();
	virtual ~CUIAnimation();

	const vector<vector<XMFLOAT3>>& GetPositions();
	const vector<vector<XMFLOAT3>>& GetRotations();
	const vector<vector<XMFLOAT3>>& GetScales();
	const vector<vector<XMFLOAT4>>& GetColors();

	virtual void Load(ifstream& in);
};
