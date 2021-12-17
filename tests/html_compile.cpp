#include "catch.hpp"

#include "html.hpp"

TEST_CASE("Compiling HTML source to template", "[html::compile]") {
	std::string_view src = R"(
<html>
	<head>
		<title>$title</title>
	</head>
	<body>
		<h3>$title</h3>
		$main
	</body>
</html>
)";

	auto html = html::compile(src);

	SECTION("Validity") {
		REQUIRE(html.insertionPoints.size() == 3);
		REQUIRE(html.insertionPoints[0].contents == "title");
		REQUIRE(html.insertionPoints[1].contents == "title");
		REQUIRE(html.insertionPoints[2].contents == "main");
	}
}
