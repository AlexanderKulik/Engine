#pragma once

#undef min
#undef max

class OBB
{
public:
	OBB()
	{}

	void								GetPoints(DirectX::SimpleMath::Vector3 points[]) const;

	bool								IsValid() const;

private:
	DirectX::SimpleMath::Matrix			m_transform;
};