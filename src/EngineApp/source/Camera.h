#pragma once

#include "Frustum.h"

class Camera
{
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Quaternion = DirectX::SimpleMath::Quaternion;
	using Matrix = DirectX::SimpleMath::Matrix;

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

	void						SetPosition(const Vector3& newPos);
	void						SetRotation(const Quaternion& newRot);
	void						SetProjectionData(float projectionData);

	void						UpdateFrustum();

	Matrix						GetViewTransform() const;
	Matrix						GetProjectionTransform() const;
	Matrix						GetViewProjectionTransform() const;

	const Frustum&				GetFrustum() const;
	const Vector3&				GetPosition() const;
	const Quaternion&			GetRotation() const;
	float						GetProjectionData() const;
	bool						IsPerspective() const;

private:
	Frustum						m_frustum;
	Vector3						m_position;
	Quaternion					m_rotation;
	float						m_aspectRatio;
	float						m_projectionData;
	float						m_nearZ;
	float						m_farZ;
	bool						m_isPerspective;
};

inline void Camera::SetPosition(const Vector3& newPos)
{
	m_position = newPos;
}

inline void Camera::SetRotation(const Quaternion& newRot)
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

inline const Camera::Vector3& Camera::GetPosition() const
{ 
	return m_position; 
}

inline const Camera::Quaternion& Camera::GetRotation() const
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