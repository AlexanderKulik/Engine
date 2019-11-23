#include "gtest/gtest.h"
#include "foo.h"

TEST(TestSuite1, Test1)
{
	int res = foo();
	EXPECT_EQ(res, 2106);
}

TEST(TestSuite1, Test2)
{
	int res = foo();
	EXPECT_EQ(res, 2106);

	//std::array<std::tuple<DirectX::SimpleMath::Vector3, float, CullResult>, 11> testSpheres =
	//{
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, CullResult::INSIDE),			//
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, -100.0f }, 1.0f, CullResult::OUTSIDE),	// behind camera
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, 600.0f }, 1.0f, CullResult::OUTSIDE),		// too far
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 100.0f, 0.0f, 0.0f }, 1.0f, CullResult::OUTSIDE),		// right side
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ -100.0f, 0.0f, 0.0f }, 1.0f, CullResult::OUTSIDE),	// left side
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 100.0f, 0.0f }, 1.0f, CullResult::OUTSIDE),		// top side
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, -100.0f, 0.0f }, 1.0f, CullResult::OUTSIDE),	// right side
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 5.0f, 5.0f }, 1.0f, CullResult::INTERSECTS),	// intersection with top
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, -7.0f, 5.0f }, 4.0f, CullResult::INTERSECTS),	// intersection with btm
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, 500.0f }, 10.0f, CullResult::INTERSECTS),	// intersection with far
	//	std::make_tuple(DirectX::SimpleMath::Vector3{ 0.0f, 0.0f, -6.0f }, 10.0f, CullResult::INTERSECTS),	// intersection with btm
	//};

	//for (auto i = 0; i < testSpheres.size(); i++)
	//{
	//	auto&& testSphere = testSpheres[i];
	//	auto result = frustum.CullSphere(std::get<0>(testSphere), std::get<1>(testSphere));
	//	assert(result == std::get<2>(testSphere));
	//}
}