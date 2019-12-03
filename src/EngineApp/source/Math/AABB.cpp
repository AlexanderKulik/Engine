#include "pch.h"

#include "AABB.h"

void AABB::AddPoint(const DirectX::SimpleMath::Vector3& point)
{
	m_minPoint = DirectX::SimpleMath::Vector3(std::min(point.x, m_minPoint.x), std::min(point.y, m_minPoint.y), std::min(point.z, m_minPoint.z));
	m_maxPoint = DirectX::SimpleMath::Vector3(std::max(point.x, m_maxPoint.x), std::max(point.y, m_maxPoint.y), std::max(point.z, m_maxPoint.z));
}

void AABB::AddAABB(const AABB& aabb)
{
	AddPoint(aabb.m_maxPoint);
	AddPoint(aabb.m_minPoint);
}

void AABB::GetPoints(DirectX::SimpleMath::Vector3 points[]) const
{
	points[0] = m_minPoint;
	points[1] = DirectX::SimpleMath::Vector3(m_minPoint.x, m_minPoint.y, m_maxPoint.z);
	points[2] = DirectX::SimpleMath::Vector3(m_maxPoint.x, m_minPoint.y, m_maxPoint.z);
	points[3] = DirectX::SimpleMath::Vector3(m_maxPoint.x, m_minPoint.y, m_minPoint.z);
	points[4] = DirectX::SimpleMath::Vector3(m_minPoint.x, m_maxPoint.y, m_minPoint.z);
	points[5] = DirectX::SimpleMath::Vector3(m_minPoint.x, m_maxPoint.y, m_maxPoint.z);
	points[6] = m_maxPoint;
	points[7] = DirectX::SimpleMath::Vector3(m_maxPoint.x, m_maxPoint.y, m_minPoint.z);
}