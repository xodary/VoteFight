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
	// 이 객체의 생성은 오로지 CAssetManager에 의해서만 일어난다.
	// 이때, 자식 클래스에서 이 클래스의 생성자를 호출해야 하므로, 접근지정자를 proteced로 설정하였다.
	CAnimation();

public:
	// 소멸자의 경우에는 SafeDelete 외부 함수를 이용하기 때문에 접근 지정자를 public으로 설정하였다.
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
