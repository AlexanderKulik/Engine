#pragma once

#include "Frustum.h"

class Camera
{
public:
	struct Perspective
	{
		float fovy;
	};

	struct Orthographic
	{
		float viewHeight;
	};

	Camera(Perspective persp, float ar, float nearZ, float farZ)
		: m_aspectRatio(ar)
		, m_nearZ(nearZ)
		, m_farZ(farZ)
		, m_isPerspective(true)
		, m_projectionData(persp.fovy)
	{}

	Camera(Orthographic ortho, float ar, float nearZ, float farZ)
		: m_aspectRatio(ar)
		, m_nearZ(nearZ)
		, m_farZ(farZ)
		, m_isPerspective(false)
		, m_projectionData(ortho.viewHeight)
	{}

	void						SetPosition(const DirectX::XMVECTOR& newPos);
	void						SetRotation(const DirectX::XMVECTOR& newRot);
	void						SetProjectionData(float projectionData);

	void						UpdateFrustum();

	DirectX::XMMATRIX			GetViewTransform() const;
	DirectX::XMMATRIX			GetProjectionTransform() const;
	DirectX::XMMATRIX			GetViewProjectionTransform() const;

	const Frustum&				GetFrustum() const;
	const DirectX::XMVECTOR&	GetPosition() const;
	const DirectX::XMVECTOR&	GetRotation() const;
	float						GetProjectionData() const;
	bool						IsPerspective() const;

private:
	Frustum						m_frustum;
	DirectX::XMVECTOR			m_position;
	DirectX::XMVECTOR			m_rotation;
	float						m_aspectRatio;
	float						m_projectionData;
	float						m_nearZ;
	float						m_farZ;
	bool						m_isPerspective;
};

inline void Camera::SetPosition(const DirectX::XMVECTOR& newPos)
{
	m_position = newPos;
}

inline void Camera::SetRotation(const DirectX::XMVECTOR& newRot)
{
	m_rotation = newRot;
}

inline void Camera::SetProjectionData(float projectionData)
{
	m_projectionData = projectionData;
}

inline const Frustum & Camera::GetFrustum() const
{
	return m_frustum;
}

inline const DirectX::XMVECTOR& Camera::GetPosition() const
{ 
	return m_position; 
}

inline const DirectX::XMVECTOR& Camera::GetRotation() const
{ 
	return m_rotation; 
}

inline float Camera::GetProjectionData() const
{
	return m_projectionData;
}

inline bool Camera::IsPerspective() const
{
	return m_isPerspective;
}