#include "foo.h"

#include <json/json.h>
#include <boost/assert.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/utility/string_view.hpp>

int testJson(const std::string& jsonVal)
{
	Json::Value root;

	Json::Reader reader;
	reader.parse(jsonVal, root);

	BOOST_ASSERT_MSG(reader.good(), "Failed to parse json!");

	boost::signals2::signal<void()> signal;
	auto&& connection = signal.connect([]()
	{
		int a = 5;
	});

	signal();

	connection.disconnect();

	signal();

	std::string str1 = "str1";
	const char* str2 = "str2";

	boost::string_view stringView = str1;
	stringView = str2;


	return static_cast<int>(root.size());
}

int foo()
{
	testJson( R"(
{
	"myVal" : 5,
	"myVal2" : 5.55,
	"myStr" : "string"
})" );

	return 2106;
}
