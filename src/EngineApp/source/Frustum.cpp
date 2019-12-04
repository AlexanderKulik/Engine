#include "pch.h"

#include "Frustum.h"
#include "Math/AABB.h"

using namespace DirectX::SimpleMath;

Vector4 GetRow(const DirectX::SimpleMath::Matrix& m, unsigned row)
{
	assert(row < 4);
	return Vector4( &m.m[row][0] );
}

Vector4 GetColumn(const DirectX::SimpleMath::Matrix& m, unsigned column)
{
	assert(column < 4);
	return Vector4( m.m[0][column], m.m[1][column], m.m[2][column], m.m[3][column] );
}

void Frustum::UpdatePlanes(const DirectX::SimpleMath::Matrix& proj)
{
	// http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
	//
	// v' = v * P
	// v' = | x y z w | * | C0 C1 C2 C3 |
	//
	// -w <= x <= w
	// -w <= y <= w
	//  0 <= z <= w

	{
		// -w <= x
		// 0 <= w + x -> 0 <= C3 + C0
		m_planes[Plane::LEFT] = GetColumn(proj, 3) + GetColumn(proj, 0);
	}

	{
		// x <= w
		// 0 <= w - x -> 0 <= C3 - C0
		m_planes[Plane::RIGHT] = GetColumn(proj, 3) - GetColumn(proj, 0);
	}

	{
		// -w <= y
		// 0 <= w + y -> 0 <= C3 + C1
		m_planes[Plane::BOTTOM] = GetColumn(proj, 3) + GetColumn(proj, 1);
	}

	{
		// y <= w
		// 0 <= w - y -> 0 <= C3 - C1
		m_planes[Plane::TOP] = GetColumn(proj, 3) - GetColumn(proj, 1);
	}

	{
		// 0 <= z -> 0 <= C2
		m_planes[Plane::NEAR] = GetColumn(proj, 2);
	}

	{
		// z <= w
		// 0 <= w - z -> 0 <= C3 - C2
		m_planes[Plane::FAR] = GetColumn(proj, 3) - GetColumn(proj, 2);
	}

	for (unsigned i = 0; i < Plane::COUNT; i++)
	{
		Vector4& plane = m_planes[i];
		const float length = std::sqrtf(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
		plane /= length;
	}

	auto projInv = proj.Invert();
	m_frustumPoints[0] = Vector3::Transform({ -1.0, -1.0f, 0.0 }, projInv);
	m_frustumPoints[1] = Vector3::Transform({ -1.0,  1.0f, 0.0 }, projInv);
	m_frustumPoints[2] = Vector3::Transform({  1.0,  1.0f, 0.0 }, projInv);
	m_frustumPoints[3] = Vector3::Transform({  1.0, -1.0f, 0.0 }, projInv);
	m_frustumPoints[4] = Vector3::Transform({ -1.0, -1.0f, 1.0 }, projInv);
	m_frustumPoints[5] = Vector3::Transform({ -1.0,  1.0f, 1.0 }, projInv);
	m_frustumPoints[6] = Vector3::Transform({  1.0,  1.0f, 1.0 }, projInv);
	m_frustumPoints[7] = Vector3::Transform({  1.0, -1.0f, 1.0 }, projInv);


}

CullResult Frustum::CullSphere(const Vector3& pos, float radius) const
{
	CullResult result = CullResult::INSIDE;

	for (unsigned i = 0; i < Plane::COUNT; i++)
	{
		const Vector4& clipPlane = m_planes[i];
		
		const Vector3 normal{ clipPlane };
		
		const float distance = normal.Dot(pos) + clipPlane.w;
		if (distance + radius < 0.0f)
		{
			return CullResult::OUTSIDE;
		}

		if (result != CullResult::INTERSECTS && distance - radius <= 0.0f)
		{
			result = CullResult::INTERSECTS;
		}
	}

	return result;
}

CullResult Frustum::CullAABB(const AABB& aabb) const
{
	std::array<Vector3, 8> points;
	aabb.GetPoints(points.data());

	CullResult result = CullResult::INSIDE;

	for (unsigned i = 0; i < Plane::COUNT; i++)
	{
		const Vector4& clipPlane = m_planes[i];

		const Vector3 normal{ clipPlane };

		unsigned pointsInside = 0;
		for (size_t v = 0; v < points.size(); v++)
		{
			const float distance = normal.Dot(points[v]) + clipPlane.w;
			if (distance > 0.0f)
			{
				pointsInside++;
			}
		}

		if (pointsInside == 0)
		{
			return CullResult::OUTSIDE;
		}

		if (result != CullResult::INTERSECTS && pointsInside < 8)
		{
			result = CullResult::INTERSECTS;
		}
	}


	return result;
}
