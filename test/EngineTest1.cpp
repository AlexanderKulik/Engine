#include "gtest/gtest.h"
#include "foo.h"

TEST(TestSuite1, Test1)
{
	int res = foo();
	EXPECT_EQ(res, 2106);
}
