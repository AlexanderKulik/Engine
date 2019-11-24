#include "pch.h"

#include "Camera.h"

Camera::Matrix Camera::GetViewTransform() const
{
	const DirectX::XMVECTOR zeroVector = DirectX::XMVectorZero();
	const DirectX::XMVECTOR unitVector = DirectX::XMVectorSplatOne();

	const DirectX::XMMATRIX cameraWorld = DirectX::XMMatrixAffineTransformation(unitVector, zeroVector, m_rotation, m_position);

	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(cameraWorld);
	return DirectX::XMMatrixInverse(&det, cameraWorld);
}

Camera::Matrix Camera::GetProjectionTransform() const
{
	if (m_isPerspective)
	{
		return DirectX::XMMatrixPerspectiveFovLH(m_projectionData, m_aspectRatio, m_nearZ, m_farZ);
	}
	else
	{
		return DirectX::XMMatrixOrthographicLH(m_projectionData, m_projectionData * m_aspectRatio, m_nearZ, m_farZ);
	}
}

Camera::Matrix Camera::GetViewProjectionTransform() const
{
	return GetViewTransform() * GetProjectionTransform();
}

void Camera::UpdateFrustum()
{
	m_frustum.UpdatePlanes( GetViewProjectionTransform() );
}