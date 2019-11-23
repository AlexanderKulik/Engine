#include "pch.h"

#include "OBB.h"

void OBB::GetPoints(DirectX::SimpleMath::Vector3 points[]) const
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