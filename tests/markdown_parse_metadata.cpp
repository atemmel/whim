#include "catch.hpp"

#include "markdown.hpp"

TEST_CASE("Parsing markdown metadata", "[md::parse]") {
	std::string_view src = R"(
---
key   = value    
title = Cool title
------
other cool text in here lmao
)";

	auto doc = md::parse(src);

	SECTION("Validity") {
		auto it = doc->meta.find("key");
		REQUIRE(it != doc->meta.end());
		REQUIRE(it->second == "value");
		it = doc->meta.find("title");
		REQUIRE(it != doc->meta.end());
		REQUIRE(it->second == "Cool title");
	}
}
