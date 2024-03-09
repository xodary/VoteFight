#pragma once
#include "Object.h"

struct LIGHT;

struct CB_CAMERA
{
	XMFLOAT4X4 m_viewMatrix;
	XMFLOAT4X4 m_projectionMatrix;

	XMFLOAT3   m_position;
};

class CCamera : public CObject
{
	friend class CCameraManager;

protected:
	CAMERA_TYPE			   m_type;

	bool				   m_isZoomIn;
	float				   m_magnification;

	D3D12_VIEWPORT         m_d3d12Viewport;
	D3D12_RECT	           m_d3d12ScissorRect;

	XMFLOAT4X4	           m_viewMatrix;
	XMFLOAT4X4	           m_projectionMatrix;

	XMFLOAT3	           m_offset;
	float		           m_speed;

	BoundingFrustum        m_frustum;

	ComPtr<ID3D12Resource> m_d3d12Buffer;
	CB_CAMERA* m_mappedData;

	CObject* m_target;
	LIGHT* m_light;

private:
	// 이 객체의 생성은 오로지 CCameraManager에 의해서만 일어난다.
	CCamera(CAMERA_TYPE type);

public:
	~CCamera();

	CAMERA_TYPE GetType();

	void SetZoomIn(bool isZoomIn);
	bool IsZoomIn();

	void SetMagnification(float magnification);
	float GetMagnification();

	void SetViewport(int topLeftX, int topLeftY, UINT width, UINT height, float minDepth, float maxDepth);
	void SetScissorRect(LONG left, LONG top, LONG right, LONG bottom);

	const XMFLOAT4X4& GetViewMatrix();
	const XMFLOAT4X4& GetProjectionMatrix();

	void SetOffset(const XMFLOAT3& offset);
	const XMFLOAT3& GetOffset();

	void SetSpeed(float speed);
	float GetSpeed();

	void SetTarget(CObject* target);
	CObject* GetTarget();

	void SetLight(LIGHT* light);
	LIGHT* GetLight();

	virtual void CreateShaderVariables();
	virtual void UpdateShaderVariables();
	virtual void ReleaseShaderVariables();

	void RSSetViewportsAndScissorRects();

	void GenerateViewMatrix(const XMFLOAT3& position, const XMFLOAT3& forward);
	void RegenerateViewMatrix();

	void GenerateBoundingFrustum();
	bool IsInBoundingFrustum(const BoundingBox& BoundingBox);

	void GenerateOrthographicsProjectionMatrix(float ViewWidth, float ViewHeight, float NearZ, float FarZ);
	void GeneratePerspectiveProjectionMatrix(float FOVAngleY, float AspectRatio, float NearZ, float FarZ);

	virtual void Update();
};
