#pragma once

#undef NEAR
#undef FAR

class AABB;

enum class CullResult
{
	INSIDE,
	INTERSECTS,
	OUTSIDE,
};

class Frustum
{
	enum Plane
	{
		LEFT = 0,
		RIGHT,
		BOTTOM,
		TOP,
		NEAR,
		FAR,

		COUNT,
	};

public:
	void								UpdatePlanes(const DirectX::SimpleMath::Matrix& worldProj);
	const DirectX::SimpleMath::Vector3*	GetFrumstumPoints() const;

	CullResult							CullSphere(const DirectX::SimpleMath::Vector3& pos, float radius) const;
	CullResult							CullAABB(const AABB& aabb) const;

private:
	DirectX::SimpleMath::Vector4		m_planes[Plane::COUNT];
	DirectX::SimpleMath::Vector3		m_frustumPoints[8];
};

inline const DirectX::SimpleMath::Vector3* Frustum::GetFrumstumPoints() const
{
	return m_frustumPoints;
}