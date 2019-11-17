#include "pch.h"

#include "AABB.h"

void AABB::AddPoint(const DirectX::SimpleMath::Vector3& point)
{
	m_minPoint = DirectX::SimpleMath::Vector3(std::min(point.x, m_minPoint.x), std::min(point.y, m_minPoint.y), std::min(point.z, m_minPoint.z));
	m_maxPoint = DirectX::SimpleMath::Vector3(std::max(point.x, m_maxPoint.x), std::max(point.y, m_maxPoint.y), std::max(point.z, m_maxPoint.z));
}

void AABB::GetPoints(DirectX::SimpleMath::Vector3 points[]) const
{
	points[0] = DirectX::SimpleMath::Vector3();
	points[1] = DirectX::SimpleMath::Vector3();
	points[2] = DirectX::SimpleMath::Vector3();
	points[3] = DirectX::SimpleMath::Vector3();
	points[4] = DirectX::SimpleMath::Vector3();
	points[5] = DirectX::SimpleMath::Vector3();
	points[6] = DirectX::SimpleMath::Vector3();
	points[7] = DirectX::SimpleMath::Vector3();
}