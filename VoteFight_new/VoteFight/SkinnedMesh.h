#pragma once
#include "Mesh.h"

class CObject;

class CSkinnedMesh : public CMesh
{
	friend class CAssetManager;

private:
	int						   m_boneCount;
							   
	ComPtr<ID3D12Resource>	   m_d3d12BoneIndexBuffer;
	ComPtr<ID3D12Resource>	   m_d3d12BoneIndexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW   m_d3d12BoneIndexBufferView;
							   
	ComPtr<ID3D12Resource>	   m_d3d12BoneWeightBuffer;
	ComPtr<ID3D12Resource>	   m_d3d12BoneWeightUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW   m_d3d12BoneWeightBufferView;
							   
	vector<XMFLOAT4X4>		   m_boneOffsetMatrixes;
	ComPtr<ID3D12Resource>	   m_d3d12BoneOffsetMatrixes;
	XMFLOAT4X4*				   m_mappedBoneOffsetMatrixes;

	// 아래 변수들은 오로지 현재 렌더링할 객체의 Animator 컴포넌트에 있는 변수를 참조하는 변수들이다.
	vector<CObject*>*		   m_boneFrameCache;

	ComPtr<ID3D12Resource>	   m_d3d12BoneTransformMatrixes;
	XMFLOAT4X4*				   m_mappedBoneTransformMatrixes;

private:
	CSkinnedMesh(const CMesh& rhs);

public:
	virtual ~CSkinnedMesh();

	void SetBoneInfo(vector<CObject*>* boneFrameCache, const ComPtr<ID3D12Resource>& d3d12BoneTransformMatrixes, XMFLOAT4X4* mappedBoneTransformMatrixes);

	void LoadSkinInfo(ifstream& in);

	virtual void UpdateShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(int subSetIndex);
};
