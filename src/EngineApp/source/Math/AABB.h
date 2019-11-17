#pragma once

#undef min
#undef max

class AABB
{
public:
	AABB()
		: m_minPoint(std::numeric_limits<float>::max())
		, m_maxPoint(std::numeric_limits<float>::min())
	{}

	void								AddPoint(const DirectX::SimpleMath::Vector3& point);

	const DirectX::SimpleMath::Vector3& GetMinPoint() const;
	const DirectX::SimpleMath::Vector3& GetMaxPoint() const;

	DirectX::SimpleMath::Vector3		GetCenter() const;
	DirectX::SimpleMath::Vector3		GetExtents() const;

	void								GetPoints(DirectX::SimpleMath::Vector3 points[]) const;

	bool								IsValid() const;

private:
	DirectX::SimpleMath::Vector3		m_minPoint;
	DirectX::SimpleMath::Vector3		m_maxPoint;
};

inline const DirectX::SimpleMath::Vector3& AABB::GetMinPoint() const
{
	return m_minPoint;
}

inline const DirectX::SimpleMath::Vector3& AABB::GetMaxPoint() const
{
	return m_maxPoint;
}

inline DirectX::SimpleMath::Vector3 AABB::GetCenter() const
{
	return (m_maxPoint + m_minPoint) * 0.5f;
}

inline DirectX::SimpleMath::Vector3 AABB::GetExtents() const
{
	return m_maxPoint - m_minPoint;
}

inline bool AABB::IsValid() const
{
	return m_minPoint.x < m_maxPoint.x 
		&& m_minPoint.y < m_maxPoint.y 
		&& m_minPoint.z < m_maxPoint.z;
}